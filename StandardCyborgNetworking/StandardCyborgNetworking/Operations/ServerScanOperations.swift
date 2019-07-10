//
//  ServerScanOperations.swift
//  Scanner
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import PromiseKit
#if canImport(ZipArchive)
import ZipArchive
#else
import SSZipArchive
#endif


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
    
    func perform(_ completion: @escaping (Result<[ServerScan]>) -> Void)
    {
        let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.scans)
        serverAPIClient.performJSONOperation(withURL: url,
                                                  httpMethod: .GET, httpBodyDict: nil,
                                                  responseObjectRootKey: "scans",
                                                  completion: completion)
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
                                                  httpBodyDict: nil,
                                                  responseObjectRootKey: nil)
        { (result: Result<SuccessResponse>) in
            switch result {
            case .success(_):
                self.dataSource.delete(self.scan)
                completion(nil)
            case .failure(let error):
                completion(error as? ServerOperationError)
            }
        }
    }
    
}

public struct ServerAddScanOperation {
    
    let scan: ServerScan
    let dataSource: ServerSyncEngineLocalDataSource
    let serverAPIClient: ServerAPIClient
    
    private struct S3UploadInfo: Codable {
        let localURL: URL
        let s3URL: URL
        let uploadHeaders: [String:Any]
        let directUploadFileKey: String

        enum CodingKeys: String, CodingKey {
            case localURL, s3URL, uploadHeaders, directUploadFileKey = "key"
        }

        init(from decoder: Decoder) throws {
            let container = try! decoder.container(keyedBy: CodingKeys.self)
            localURL = try! container.decode(URL.self, forKey: .localURL)
            s3URL = try! container.decode(URL.self, forKey: .s3URL)

            let headerData = try! container.decode(Data.self, forKey: .uploadHeaders)
            uploadHeaders = try! JSONSerialization.jsonObject(with: headerData, options: []) as! [String : Any]
            directUploadFileKey = try! container.decode(String.self, forKey: .directUploadFileKey)
        }

        func encode(to encoder: Encoder) throws {
            fatalError("Not currently supported (but implemented since our generic networking code requires objects to conform to Encodable)")
        }
    }

    public init(scan: ServerScan,
                dataSource: ServerSyncEngineLocalDataSource,
                serverAPIClient: ServerAPIClient)
    {
        self.scan = scan
        self.dataSource = dataSource
        self.serverAPIClient = serverAPIClient
    }
    
    public func perform(uploadProgress: ((Double) -> Void)?,
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
                                                 httpBodyDict: postDict,
                                                 responseObjectRootKey: nil)
            { (result: Result<S3UploadInfo>) in
                switch result {
                case .success(let uploadInfo):
                    print("Successfully created S3 file reference \(uploadInfo.directUploadFileKey)")
                    seal.fulfill(uploadInfo)
                case .failure(let error):
                    seal.reject(ServerOperationError.genericErrorString("Error creating S3 file from result: \(error.localizedDescription)"))
                }
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
                scan.uploadStatus = .uploaded
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
                                                      httpBodyDict: postDict,
                                                      responseObjectRootKey: "scan")
            { (result: Result<ServerScan>) in
                switch result {
                case .success(let scan):
                    print("Successfully created server scan with uid \(scan.key ?? "unknown")")
                    self.dataSource.update(self.scan.merge(withScan: scan))
                    seal.fulfill(())

                case .failure(let error):
                    print("Failed to get scan info from POST to \(url)")
                    seal.reject(error)
                }
            }
        }
    }
    
}

public struct ServerDownloadScanOperation {
    
    private struct S3DownloadInfo: Codable {
        let zippedScanURL: URL
        let thumbnailURL: URL?

        enum CodingKeys: String, CodingKey {
            case zippedScanURL = "fileUrl"
            case thumbnailURL = "thumbnailUrl"
        }
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
                                                      httpBodyDict: nil,
                                                      responseObjectRootKey: "scan")
            { (result: Result<S3DownloadInfo>) in
                switch result {
                case .success(let downloadInfo):
                    seal.fulfill(downloadInfo)
                case .failure(let error):
                    seal.reject(error)
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
