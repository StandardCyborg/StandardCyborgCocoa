//
//  ServerSyncEngine.swift
//  Scanner
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import PromiseKit

/** Implement these protocol methods to bridge between Server model objects
    and your own local model representation */
public protocol ServerSyncEngineLocalDataSource {
    /** If desired, requests that the client opens a write transaction */
    func beginWriteTransaction()
    /** If desired, requests that the client close a write transaction */
    func endWriteTransaction()
    
    /** There can only be one */
    var user: ServerUser { get }
    
    /** Update the single user from the specified ServerUser */
    func updateUser(_ user: ServerUser)
    
    /** Called when signing out */
    func resetUser()
    
    /** Gotta fetch em' all! Return an array of your local scans, mapped to
        ServerScan instances. */
    func allServerScans() -> [ServerScan]
    
    /** Create a record of this scan using your own model's representation */
    func add(_ scan: ServerScan)
    
    /** Update your own model's representation using the given scan */
    func update(_ scan: ServerScan)
    
    /** Called when a PLY file finishes downloading to the given local file URL */
    func didDownloadPLYFile(for scan: ServerScan, to url: URL)
    
    /** Delete the your own model's representation of the given scan */
    func delete(_ scan: ServerScan)
    
    /** Return the full local file URL of where the scan's PLY file should be
        expected on disk. If the file does not exist yet, return a URL of where
        it should be downloaded to. */
    func localPLYURL(for scan: ServerScan) -> URL?
    
    /** Return the full local file URL of where the scan's thumbnail file should
        be expected on disk. If the file does not exist yet, return a URL of where
        it should be downloaded to. */
    func localThumbnailURL(for scan: ServerScan) -> URL?
}

public class ServerSyncEngine {
    
    public static let syncDidStartNotification = NSNotification.Name("SyncDidStart")
    public static let syncDidEndNotification = NSNotification.Name("SyncDidEnd")
    public static let syncingCountDidChangeNotification = NSNotification.Name("SyncingCountDidChange")
    
    private let _serverAPIClient: ServerAPIClient
    private let _dataSource: ServerSyncEngineLocalDataSource
    
    public init(serverAPIClient: ServerAPIClient, dataSource: ServerSyncEngineLocalDataSource) {
        self._serverAPIClient = serverAPIClient
        self._dataSource = dataSource
    }
    
    /** That is, whether the current user is signed in and the session is still valid */
    public var isSessionValid: Bool {
        return _dataSource.user.key != nil && _serverAPIClient.isValid
    }
    
    /** Performs a full sync, start to finish */
    public func performSync(_ completion: @escaping (ServerOperationError?) -> Void) {
        guard isSessionValid else {
            // We consider this to not be an error case
            completion(nil)
            return
        }
        
        isSyncing = true
        
        ServerFetchScansOperation(dataSource: _dataSource, serverAPIClient: _serverAPIClient)
        .perform { error, scans in
            guard let scans = scans else {
                completion(error)
                return
            }
            
            var scansToUpload: [ServerScan] = []
            var scansToDownload: [ServerScan] = []
            try? self._reconcileRemoteScansWithLocal(scans, scansToUpload: &scansToUpload, scansToDownload: &scansToDownload)
            
            self._upload(scansToUpload, download: scansToDownload) { error in
                self.isSyncing = false
                completion(error)
            }
        }
    }
    
    /** True while a sync is in progress */
    public private(set) var isSyncing: Bool = false {
        didSet {
            print("Sync \(isSyncing ? "began" : "finished")")
            let name = isSyncing ? ServerSyncEngine.syncDidStartNotification : ServerSyncEngine.syncDidEndNotification
            NotificationCenter.default.post(name: name, object: self)
        }
    }
    
    /** The number of scans currently being uploaded and downloaded */
    public private(set) var syncingCount: Int = 0 {
        didSet {
            print("Syncing count changed to \(syncingCount)")
            NotificationCenter.default.post(name: ServerSyncEngine.syncingCountDidChangeNotification, object: self)
        }
    }
    
    /** Provides the current upload status for a given scan */
    public func uploadStatus(for scan: ServerScan) -> ServerScan.UploadStatus {
        if let uploadStatus = _uploadStatusPerScanUUID[scan.localUUID] {
            return uploadStatus
        } else if let status = scan.uploadStatus {
            return status
        } else {
            return .notUploaded
        }
    }
    
    // MARK: - Private
    
    private var _uploadStatusPerScanUUID: [UUID: ServerScan.UploadStatus] = [:]
    
    private func _setSyncStatus(_ status: ServerScan.UploadStatus, for scan: ServerScan) {
        DispatchQueue.main.async {
            NotificationCenter.default.post(name: ServerSyncEngine.syncDidStartNotification, object: self)
        }
    }
    
    private func _reconcileRemoteScansWithLocal(_ remoteScans: [ServerScan],
                                                scansToUpload: inout [ServerScan],
                                                scansToDownload: inout [ServerScan])
    throws
    {
        let localScans = _dataSource.allServerScans()
        
        let remoteScansByServerKey: [String:ServerScan] = remoteScans.reduce(into: [:]) { result, scan in
            if let key = scan.key {
                result[key] = scan
            }
        }
        
        // Scans available locally but not remotely should be uploaded
        //  -> scan has a documentsRelativePLYPath, but no uid
        // Scans available remotely but not locally should be downloaded
        //  -> scan has a uid, but no documentsRelativePLYPath
        // Scans available both locally and remotely should be updated from the server
        // Deleting locally nor remotely should not happen during a sync
        
        let fileManager = FileManager.default
        var unvisitedServerKeys = Set(remoteScansByServerKey.keys)
        
        for localScan in localScans {
            guard let key = localScan.key else {
                // Available locally, but not remotely
                scansToUpload.append(localScan)
                continue
            }
            
            unvisitedServerKeys.remove(key)
            
            // if let remoteScan = remoteScansByServerKey[key] {
            //     Available both locally and locally, but there's nothing to update
            // }
            
            // Known about locally, but scan data doesn't exist (e.g. failed to download earlier)
            if let localPLYURL = _dataSource.localPLYURL(for: localScan),
                fileManager.fileExists(atPath: localPLYURL.path)
            {
                // File exists locally, all good
            } else {
                scansToDownload.append(localScan)
            }
        }
        
        _dataSource.beginWriteTransaction()
        
        for key in unvisitedServerKeys {
            // Available remotely but not locally
            let scan = remoteScansByServerKey[key]!
            _dataSource.add(scan)
            scansToDownload.append(scan)
        }
        
        _dataSource.endWriteTransaction()
    }
    
    private func _upload(_ scansToUpload: [ServerScan], download scansToDownload: [ServerScan], completion: @escaping (ServerOperationError?) -> Void) {
        let uploadOperations: [ServerAddScanOperation] = scansToUpload.map {
            ServerAddScanOperation(scan: $0, dataSource: self._dataSource, serverAPIClient: self._serverAPIClient)
        }
        
        let downloadOperations: [ServerDownloadScanOperation] = scansToDownload.map {
            ServerDownloadScanOperation(scan: $0, dataSource: self._dataSource, serverAPIClient: self._serverAPIClient)
        }
        
        let uploadPromises: [Promise<Void>] = uploadOperations.map { operation in
            Promise<Void> { seal in
                let plyURL = self._dataSource.localPLYURL(for: operation.scan)
                print("Uploading scan from \(plyURL?.path ?? "unset PLY URL")")
                
                self._uploadStatusPerScanUUID[operation.scan.localUUID] = .uploading(percentComplete: 0)
                
                operation.perform(uploadProgress: { percentComplete in
                    self._uploadStatusPerScanUUID[operation.scan.localUUID] = .uploading(percentComplete: percentComplete)
                }, completion: { error in
                    if let error = error { print("Error uploading scan from \(plyURL?.path ?? "unkown PLY path"): \(error)") }
                    else { print("Uploaded scan from \(plyURL!.path)") }
                    
                    self._uploadStatusPerScanUUID[operation.scan.localUUID] = .uploaded
                    self.syncingCount -= 1
                    seal.resolve(error)
                })
            }
        }
        
        let downloadPromises: [Promise<Void>] = downloadOperations.map { operation in
            Promise<Void> { seal in
                print("Downloading scan \(operation.scan.key ?? "unkown server key")")
                
                operation.perform(downloadProgress: { percentComplete in
                    self._uploadStatusPerScanUUID[operation.scan.localUUID] = .uploading(percentComplete: percentComplete)
                }, completion: { error in
                    if let error = error { print("Error downloading scan from \(operation.scan.key ?? "unkown server key"): \(error)") }
                    else { print("Downloaded scan to \(self._dataSource.localPLYURL(for: operation.scan)!.path)") }
                    
                    self._uploadStatusPerScanUUID[operation.scan.localUUID] = nil
                    self.syncingCount -= 1
                    seal.resolve(error)
                })
            }
        }
        
        print("Uploading \(uploadPromises.count) scans and downloading \(downloadPromises.count) scans")
        let allPromises = uploadPromises + downloadPromises
        self.syncingCount = allPromises.count
        
        var chainedPromise = Promise()
        for promise in allPromises {
            chainedPromise = chainedPromise.then { promise }
        }
        _ = chainedPromise.done { completion(nil) }
        .catch { error in
            let serverError = error as? ServerOperationError ?? ServerOperationError.genericError(error)
            completion(serverError)
        }
    }
    
}
