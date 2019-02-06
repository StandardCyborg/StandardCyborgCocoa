//
//  ServerScanOperations.swift
//  Scanner
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import PromiseKit
import ZipArchive

private struct ClientAPIPath {
    static let scans = "scans"
    static let s3Files = "direct_uploads"
}

public struct ServerFetchScansOperation {
    
    let dataSource: ServerSyncEngineLocalDataSource
    let serverAPIClient: ServerAPIClient
    
    public init(dataSource: ServerSyncEngineLocalDataSource, serverAPIClient: ServerAPIClient) {
        self.dataSource = dataSource
        self.serverAPIClient = serverAPIClient
    }
    
    func perform(_ completion: @escaping (ServerOperationError?, Array<ServerScan>?) -> Void)
    {
        let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.scans)
        
        serverAPIClient.performJSONOperation(withURL: url,
                                             httpMethod: .GET,
                                             httpBodyDict: nil)
        { (error: ServerOperationError?, resultJSONObject: Any?) in
            guard error == nil else { return completion(error, nil) }
            
            guard let resultDict = resultJSONObject as? [String: Any],
                  let scansDict = resultDict["scans"] as? [[String: Any]]
            else {
                let mapError = ServerOperationError.genericErrorString(
                    "Failed to map root JSON results to scans")
                completion(mapError, nil)
                return
            }
            
            guard let remoteScans = ServerScan.scans(fromJSONObject: scansDict) else {
                let mapError = ServerOperationError.genericErrorString(
                    "Failed to map servers scans to local scans: \(String(describing: resultJSONObject))")
                completion(mapError, nil)
                return
            }
            
            completion(nil, remoteScans)
        }
    }
    
}

public struct ServerDeleteScanOperation {
    
    let scan: ServerScan
    let dataSource: ServerSyncEngineLocalDataSource
    let serverAPIClient: ServerAPIClient
    
    public init(scan: ServerScan,
                dataSource: ServerSyncEngineLocalDataSource,
                serverAPIClient: ServerAPIClient)
    {
        self.scan = scan
        self.dataSource = dataSource
        self.serverAPIClient = serverAPIClient
    }

    
    public func perform(_ completion: @escaping (ServerOperationError?) -> Void)
    {
        guard let key = scan.key else {
            completion(ServerOperationError.genericErrorString("Could not delete scan with no server key"))
            return
        }
        
        let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.scans)
                .appendingPathComponent(key)
        
        serverAPIClient.performJSONOperation(withURL: url,
                                             httpMethod: .DELETE,
                                             httpBodyDict: nil)
        { (error: ServerOperationError?, resultJSONObject: Any?) in
            guard error == nil else {
                completion(error)
                return
            }
            
            self.dataSource.delete(self.scan)
            
            completion(nil)
        }
    }
    
}

public struct ServerAddScanOperation {
    
    let scan: ServerScan
    let dataSource: ServerSyncEngineLocalDataSource
    let serverAPIClient: ServerAPIClient
    
    private struct S3UploadInfo {
        let localURL: URL
        let s3URL: URL
        let uploadHeaders: [String:Any]
        let directUploadFileKey: String
    }
    
    func perform(uploadProgress: ((Double) -> Void)?,
                 completion: @escaping (ServerOperationError?) -> Void)
    {
        let thumbnailURL = self.dataSource.localThumbnailURL(for: self.scan)
        var scanZipUploadInfo: S3UploadInfo!
        var thumbnailUploadInfo: S3UploadInfo?
        
        var promise =
        firstly {
            self._zipScanFile(for: scan)
        }.then { scanZipURL in
            self._createS3FileReference(for: scanZipURL, remoteFilename: "scan.ply.zip")
        }.then { uploadInfo in
            self._uploadScanFileToS3(uploadInfo, progressHandler: uploadProgress)
        }.map { uploadInfo in
            scanZipUploadInfo = uploadInfo
        }
        
        // This is how you effectively branch in PromiseKit
        if let thumbnailURL = thumbnailURL { promise = promise
            .then {
                self._createS3FileReference(for: thumbnailURL, remoteFilename: "thumbnail.jpeg")
            }.then { uploadInfo in
                self._uploadScanThumbnailToS3(uploadInfo)
            }.map { uploadInfo in
                thumbnailUploadInfo = uploadInfo
            }
        }
        
        promise = promise.then { _ in
            self._createSCServerScan(scanZipUploadInfo: scanZipUploadInfo, thumbnailUploadInfo: thumbnailUploadInfo)
        }
        
        promise.done {
            completion(nil)
        }.catch { error in
            let serverError = error as? ServerOperationError ?? ServerOperationError.genericError(error)
            completion(serverError)
        }
    }
    
    private func _zipScanFile(for scan: ServerScan) -> Promise<URL> {
        return Promise { seal in
            let plyURL = self.dataSource.localPLYURL(for: self.scan)
            
            guard
                let plyPath = plyURL?.path,
                FileManager.default.fileExists(atPath: plyPath)
            else {
                seal.reject(ServerOperationError.genericErrorString(
                    "Told to upload scan, but the file does not exist at \(plyURL?.path ?? "(none specified)")"))
                return
            }
            
            // TODO: Leverage PromiseKit better in dealing with these queues
            DispatchQueue.global(qos: DispatchQoS.QoSClass.utility).async {
                // Compress the files before upload
                let tempZipURL = URL(fileURLWithPath: NSTemporaryDirectory())
                    .appendingPathComponent(NSUUID().uuidString)
                    .appendingPathExtension("zip")
                let pathsToZip = [plyPath]
                
                let zipSuccess = SSZipArchive.createZipFile(atPath: tempZipURL.path, withFilesAtPaths: pathsToZip)
                
                DispatchQueue.main.async {
                    if zipSuccess {
                        seal.fulfill(tempZipURL)
                    } else {
                        seal.reject(ServerOperationError.genericErrorString(
                            "Failed to zip files at \(pathsToZip) to \(tempZipURL))"))
                    }
                }
            }
        }
    }
    
    private func _createS3FileReference(for localFileURL: URL, remoteFilename: String) -> Promise<S3UploadInfo> {
        return Promise { seal in
            guard let fileMD5 = MD5File(url: localFileURL) else {
                seal.reject(ServerOperationError.genericErrorString("Couldn't calculate MD5 for \(localFileURL)"))
                return
            }
            
            guard
                let fileAttributes = try? FileManager.default.attributesOfItem(atPath: localFileURL.path),
                let fileSize = fileAttributes[FileAttributeKey.size] as? UInt64
            else {
                seal.reject(ServerOperationError.genericErrorString("Couldn't get file size for \(localFileURL)"))
                return
            }
            
            let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.s3Files)
            let postDict = [
                "blob": [
                    "content_type": "text/plain",
                    "filename": remoteFilename,
                    "checksum": fileMD5.base64EncodedString(),
                    "byte_size": fileSize,
                ]
            ]
            
            print("Creating S3 file reference for \(localFileURL)")
            serverAPIClient.performJSONOperation(withURL: url,
                                                 httpMethod: HTTPMethod.POST,
                                                 httpBodyDict: postDict)
            { (error: ServerOperationError?, jsonObject: Any?) in
                guard error == nil else {
                    return seal.reject(error!)
                }
                
                guard
                    let jsonDict = jsonObject as? [String: Any],
                    let fileKey = jsonDict["key"] as? String,
                    let directUploadDict = jsonDict["direct_upload"] as? [String:Any],
                    let uploadURLString = directUploadDict["url"] as? String,
                    let uploadURL = URL(string: uploadURLString),
                    let uploadHeaders = directUploadDict["headers"] as? [String:Any]
                else {
                    seal.reject(ServerOperationError.genericErrorString("Error creating S3 file from result \(String(describing: jsonObject))"))
                    return
                }
                
                print("Successfully created S3 file reference \(fileKey)")
                let uploadInfo = S3UploadInfo(localURL: localFileURL,
                                              s3URL: uploadURL,
                                              uploadHeaders: uploadHeaders,
                                              directUploadFileKey: fileKey)
                seal.fulfill(uploadInfo)
            }
        }
    }
    
    private func _uploadScanFileToS3(_ uploadInfo: S3UploadInfo, progressHandler: ((Double) -> Void)?) -> Promise<S3UploadInfo> {
        return Promise<S3UploadInfo> { seal in
            print("Uploading scan file to S3 for \(uploadInfo.directUploadFileKey)")
            serverAPIClient.performDataUploadOperation(withURL: uploadInfo.s3URL,
                                                       httpMethod: HTTPMethod.PUT,
                                                       dataURL: uploadInfo.localURL,
                                                       extraHeaders: uploadInfo.uploadHeaders,
                                                       progressHandler: progressHandler)
            { error in
                guard error == nil else { return seal.reject(error!) }
                
                try? FileManager.default.removeItem(at: uploadInfo.localURL)
                
                var scan = self.scan
                scan.uploadedAt = Date()
                self.dataSource.update(scan)
                
                print("Successfully uploaded scan file to S3")
                seal.fulfill(uploadInfo)
            }
        }
    }
    
    private func _uploadScanThumbnailToS3(_ uploadInfo: S3UploadInfo) -> Promise<S3UploadInfo> {
        return Promise<S3UploadInfo> { seal in
            print("Uploading scan thumbnail to S3 for \(uploadInfo.directUploadFileKey)")
            serverAPIClient.performDataUploadOperation(withURL: uploadInfo.s3URL,
                                                       httpMethod: HTTPMethod.PUT,
                                                       dataURL: uploadInfo.localURL,
                                                       extraHeaders: uploadInfo.uploadHeaders,
                                                       progressHandler: nil)
            { error in
                if error == nil { print("Successfully uploaded scan thumbnail to S3") }
                
                seal.resolve(uploadInfo, error)
            }
        }
    }
    
    private func _createSCServerScan(scanZipUploadInfo: S3UploadInfo, thumbnailUploadInfo: S3UploadInfo?) -> Promise<Void> {
        return Promise { seal in
            guard scan.key == nil else {
                seal.reject(ServerOperationError.genericErrorString("This scan already has a server key: \(scan.key!)"))
                return
            }
            
            let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.scans)
            
            var postDict: [String: Any] = ["file_key": scanZipUploadInfo.directUploadFileKey]
            if let thumbnailKey = thumbnailUploadInfo?.directUploadFileKey {
                postDict["thumbnail_key"] = thumbnailKey
            }
            
            serverAPIClient.performJSONOperation(withURL: url,
                                                 httpMethod: .POST,
                                                 httpBodyDict: postDict)
            { (error: ServerOperationError?, jsonObject: Any?) in
                guard error == nil else { return seal.reject(error!) }
                
                if  let jsonDict = jsonObject as? [String:Any],
                    let scanDict = jsonDict["scan"] as? [String:Any]
                {
                    print("Successfully created server scan with uid \(scanDict["uid"] ?? "unknown")")
                    var scan = self.scan
                    scan.update(fromJSONObject: scanDict)
                    self.dataSource.update(scan)
                } else {
                    print("Failed to get scan info from POST to \(url)")
                }
                
                seal.fulfill(())
            }
        }
    }
    
}

public struct ServerDownloadScanOperation {
    
    private struct S3DownloadInfo {
        let zippedScanURL: URL
        let thumbnailURL: URL?
    }
    
    let scan: ServerScan
    let dataSource: ServerSyncEngineLocalDataSource
    let serverAPIClient: ServerAPIClient
    
    // DEV: This can be called from any stage in an interrupted upload,
    //      e.g. if the SC server scan was created, but the S3 file reference was not yet,
    //      or if the S3 upload timed out previously
    func perform(downloadProgress: ((Double) -> Void)?,
                 completion: @escaping (ServerOperationError?) -> Void)
    {
        firstly {
            self._getScanS3FileAndThumbnailURLs()
        }.then { downloadInfo in
            self._downloadScanFileFromS3(downloadInfo: downloadInfo, progressHandler: downloadProgress)
        }.then { downloadInfo in
            self._downloadThumbnailFromS3(at: downloadInfo.thumbnailURL)
        }.done {
            completion(nil)
        }.catch { error in
            let serverError = error as? ServerOperationError ?? ServerOperationError.genericError(error)
            completion(serverError)
        }
    }
    
    private func _getScanS3FileAndThumbnailURLs() -> Promise<S3DownloadInfo>
    {
        return Promise { seal in
            guard let key = scan.key else {
                return seal.reject(ServerOperationError.genericErrorString("This scan doesn't have a server key"))
            }
            
            print("Getting S3 file URL for \(key)")
            let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.scans).appendingPathComponent(key)
            
            serverAPIClient.performJSONOperation(withURL: url,
                                                 httpMethod: .GET,
                                                 httpBodyDict: nil)
            { (error: ServerOperationError?, jsonObject: Any?) in
                if error == nil,
                    let json = jsonObject as? [String:Any],
                    let scan = json["scan"] as? [String:Any],
                    let fileURLString = scan["file_url"] as? String,
                    let s3FileURL = fileURLString.toURL()
                {
                    let thumbnailURLString = scan["thumbnail_url"] as? String
                    let s3ThumbnailURL = thumbnailURLString?.toURL()
                    let downloadInfo = S3DownloadInfo(zippedScanURL: s3FileURL, thumbnailURL: s3ThumbnailURL)
                    
                    seal.fulfill(downloadInfo)
                } else {
                    seal.reject(error!)
                }
            }
        }
    }
    
    private func _downloadScanFileFromS3(downloadInfo: S3DownloadInfo, progressHandler: ((Double) -> Void)? = nil) -> Promise<S3DownloadInfo>
    {
        return Promise<S3DownloadInfo> { seal in
            let tempURL = URL(fileURLWithPath: NSTemporaryDirectory())
                .appendingPathComponent(NSUUID().uuidString)
                .appendingPathExtension("zip")
            
            print("Downloading file from S3 for \(scan.key ?? "nil server key")")
            serverAPIClient.performDataDownloadOperation(withURL: downloadInfo.zippedScanURL,
                                                         httpMethod: HTTPMethod.GET,
                                                         destinationURL: tempURL,
                                                         extraHeaders: nil,
                                                         progressHandler: progressHandler)
            { error in
                guard error == nil else { return seal.reject(error!) }
                
                guard let destinationPLYURL = self.dataSource.localPLYURL(for: self.scan) else {
                    return seal.reject(ServerOperationError.genericErrorString("Data source didn't return a local PLY URL for scan \(self.scan.key ?? "unknown key")"))
                }
                
                DispatchQueue.global(qos: DispatchQoS.QoSClass.utility).async {
                    do {
                        _ = try ServerDownloadScanOperation._unzipAndMovePLYFile(at: tempURL, to: destinationPLYURL.path)
                        
                        self.dataSource.didDownloadPLYFile(for: self.scan, to: destinationPLYURL)
                        
                        DispatchQueue.main.async {
                            seal.fulfill(downloadInfo)
                        }
                    } catch {
                        DispatchQueue.main.async {
                            seal.reject(error)
                        }
                    }
                }
            }
        }
    }
    
    private static func _unzipAndMovePLYFile(at url: URL, to destinationPath: String) throws -> String? {
        let fileManager = FileManager.default
        let unzipWorkspacePath = NSTemporaryDirectory().appendingFormat("CaptureUnzipWorkspace-%@", destinationPath.toURL()!.lastPathComponent)
        
        if fileManager.fileExists(atPath: unzipWorkspacePath) {
            try fileManager.removeItem(atPath: unzipWorkspacePath)
        }
        
        try fileManager.createDirectory(atPath: unzipWorkspacePath, withIntermediateDirectories: false, attributes: nil)
        
        SSZipArchive.unzipFile(atPath: url.path, toDestination: unzipWorkspacePath)
        
        guard
            let contents = try? fileManager.contentsOfDirectory(atPath: unzipWorkspacePath),
            let plyFilename = contents.first(where: { $0.hasSuffix(".ply") })
        else { return nil }
        
        let plyPath = (unzipWorkspacePath as NSString).appendingPathComponent(plyFilename)
        
        if fileManager.fileExists(atPath: destinationPath) {
            try fileManager.removeItem(atPath: destinationPath)
        }
        
        try fileManager.moveItem(atPath: plyPath, toPath: destinationPath)
        
        return plyFilename
    }
    
    private func _downloadThumbnailFromS3(at url: URL?) -> Promise<Void> {
        return Promise<Void> { seal in
            // If there's no thumbnail URL, treat it as a non-error for now
            guard let url = url else { return seal.fulfill(()) }
            
            guard let destinationURL = self.dataSource.localThumbnailURL(for: self.scan) else {
                return seal.reject(ServerOperationError.genericErrorString(
                    "No local thumbnail URL specified for scan \(self.scan.key ?? "unknown")")
                )
            }
            
            let tempURL = URL(fileURLWithPath: NSTemporaryDirectory())
                .appendingPathComponent(NSUUID().uuidString)
                .appendingPathExtension("jpeg")
            
            print("Downloading thumbnail from S3 for \(scan.key ?? "nil server key")")
            serverAPIClient.performDataDownloadOperation(withURL: url,
                                                         httpMethod: HTTPMethod.GET,
                                                         destinationURL: tempURL,
                                                         extraHeaders: nil,
                                                         progressHandler: nil)
            { error in
                guard error == nil else { return seal.reject(error!) }
                
                try? FileManager.default.moveItem(at: tempURL, to: destinationURL)
                
                seal.fulfill(())
            }
        }
    }
    
}

extension String {
    func toURL() -> URL? {
        return URL(string: self)
    }
}
