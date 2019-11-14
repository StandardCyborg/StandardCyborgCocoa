//
//  ServerScanOperations.swift
//  StandardCyborgNetworking
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import PromiseKit
import Zip

private struct ClientAPIPath {
    static let scans = "scans"
    static let s3Files = "direct_uploads"
}

public class ServerFetchScansOperation: ServerOperation {
    
    func perform(_ completion: @escaping (Result<[ServerScan]>) -> Void)
    {
        let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.scans)
        serverAPIClient.performJSONOperation(withURL: url,
                                                  httpMethod: .GET, httpBodyDict: nil,
                                                  responseObjectRootKey: "scans",
                                                  completion: completion)
    }
    
}

public class ServerDeleteScanOperation: ServerOperation {
    
    let scan: ServerScan
    
    public init(scan: ServerScan,
                dataSource: ServerSyncEngineLocalDataSource,
                serverAPIClient: ServerAPIClient)
    {
        self.scan = scan
        super.init(dataSource: dataSource, serverAPIClient: serverAPIClient)
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

public class ServerAddScanOperation: ServerOperation {
    
    let scan: ServerScan
    
    public init(scan: ServerScan,
                dataSource: ServerSyncEngineLocalDataSource,
                serverAPIClient: ServerAPIClient)
    {
        self.scan = scan
        super.init(dataSource: dataSource, serverAPIClient: serverAPIClient)
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
        }.then { localURL, uploadInfo in
            self._uploadScanFileToS3(localURL: localURL, uploadInfo: uploadInfo, progressHandler: uploadProgress)
        }.map { uploadInfo in
            scanZipUploadInfo = uploadInfo
        }
        
        // This is how you effectively branch in PromiseKit
        if let thumbnailURL = thumbnailURL { promise = promise
            .then {
                self._createS3FileReference(for: thumbnailURL, remoteFilename: "thumbnail.jpeg")
            }.then { localURL, uploadInfo in
                self._uploadScanThumbnailToS3(localURL: localURL, uploadInfo: uploadInfo)
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
                let pathsToZip: [URL] = [URL(fileURLWithPath: plyPath)]
                
                var zipError: Error?
                do {
                    try _ = Zip.quickZipFiles(pathsToZip, fileName: tempZipURL.path)
                } catch {
                    zipError = error
                }
                
                DispatchQueue.main.async {
                    if let zipError = zipError {
                        seal.reject(ServerOperationError.genericErrorString(
                            "Failed to zip files at \(pathsToZip) to \(tempZipURL): \(zipError)"))
                    } else {
                        seal.fulfill(tempZipURL)
                    }
                }
            }
        }
    }
    
    private func _uploadScanFileToS3(localURL: URL, uploadInfo: S3UploadInfo, progressHandler: ((Double) -> Void)?) -> Promise<S3UploadInfo> {
        return Promise<S3UploadInfo> { seal in
            print("Uploading scan file to S3 for \(uploadInfo.directUploadFileKey)")
            serverAPIClient.performDataUploadOperation(withURL: uploadInfo.url,
                                                       httpMethod: HTTPMethod.PUT,
                                                       dataURL: localURL,
                                                       extraHeaders: uploadInfo.uploadHeaders,
                                                       progressHandler: progressHandler)
            { error in
                guard error == nil else { return seal.reject(error!) }
                
                try? FileManager.default.removeItem(at: localURL)
                
                var scan = self.scan
                scan.uploadedAt = Date()
                scan.uploadStatus = .uploaded
                self.dataSource.update(scan)
                
                print("Successfully uploaded scan file to S3")
                seal.fulfill(uploadInfo)
            }
        }
    }
    
    private func _uploadScanThumbnailToS3(localURL: URL, uploadInfo: S3UploadInfo) -> Promise<S3UploadInfo> {
        return Promise<S3UploadInfo> { seal in
            print("Uploading scan thumbnail to S3 for \(uploadInfo.directUploadFileKey)")
            serverAPIClient.performDataUploadOperation(withURL: uploadInfo.url,
                                                       httpMethod: HTTPMethod.PUT,
                                                       dataURL: localURL,
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
            let scanToUpload = ServerScan(scanUploadInfo: scanZipUploadInfo, thumbnailUploadInfo: thumbnailUploadInfo)

            // Convert the scan object to a dictionary by encoding it ServerScan => Data => Dictionary.
            // We then embed that dictionary inside a root "scan" object since that's what the server expects.
            let encoder = JSONEncoder()
            encoder.keyEncodingStrategy = .convertToSnakeCase
            let scan = try! JSONSerialization.jsonObject(with: try! encoder.encode(scanToUpload), options: []) as! [AnyHashable : Any]

            serverAPIClient.performJSONOperation(withURL: url,
                                                 httpMethod: .POST,
                                                 httpBodyDict: ["scan" : scan],
                                                 responseObjectRootKey: "scan")
            { (result: Result<ServerScan>) in
                switch result {
                    
                case .success(var scan):
                    print("Successfully created server scan with uid \(scan.key ?? "unknown")")
                    scan.localUUID = self.scan.localUUID
                    scan.uploadStatus = self.scan.uploadStatus
                    self.dataSource.update(scan)
                    seal.fulfill(())

                case .failure(let error):
                    print("Failed to get scan info from POST to \(url)")
                    seal.reject(error)
                }
            }
        }
    }
    
}

public class ServerDownloadScanOperation: ServerOperation {
    
    private struct S3DownloadInfo: Codable {
        let zippedScanURL: URL
        let thumbnailURL: URL?

        enum CodingKeys: String, CodingKey {
            case zippedScanURL = "fileUrl"
            case thumbnailURL = "thumbnailUrl"
        }
    }
    
    let scan: ServerScan
    
    public init(scan: ServerScan,
                dataSource: ServerSyncEngineLocalDataSource,
                serverAPIClient: ServerAPIClient)
    {
        self.scan = scan
        super.init(dataSource: dataSource, serverAPIClient: serverAPIClient)
    }
    
    
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
        
        try Zip.unzipFile(url, destination: URL(fileURLWithPath: unzipWorkspacePath), overwrite: true, password: nil)
        
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

fileprivate extension ServerScan {
    init(scanUploadInfo: S3UploadInfo, thumbnailUploadInfo: S3UploadInfo?) {
        self.localUUID = UUID()
        self.uploadStatus = .notUploaded
        self.key = nil
        self.createdAt = nil
        self.uploadedAt = nil
        self.tagList = []
        self.attachments = [
            ServerScanAttachment(scanUploadInfo: scanUploadInfo, thumbnailUploadInfo: thumbnailUploadInfo)
        ]
    }
}

fileprivate extension ServerScanAttachment {
    init(scanUploadInfo: S3UploadInfo, thumbnailUploadInfo: S3UploadInfo?) {
        self.fileKey = scanUploadInfo.directUploadFileKey
        self.thumbnailKey = thumbnailUploadInfo?.directUploadFileKey
        self.kind = nil
        self.publiclyDownloadable = false
        self.publiclyVisible = false
        self.metadata = [:]

        self.createdAt = nil
        self.uploadedAt = nil
        self.fileUrl = nil
        self.thumbnailUrl = nil
        self.teamUid = nil
        self.collectionUid = nil
    }

}
