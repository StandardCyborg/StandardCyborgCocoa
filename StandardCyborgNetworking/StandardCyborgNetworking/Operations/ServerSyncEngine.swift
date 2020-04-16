//
//  ServerSyncEngine.swift
//  StandardCyborgNetworking
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
    
    /** Return an array of scene files that have not yet been uploaded
        to the server. */
    func localSceneFiles() -> [LocalSceneFile]
    
    /** Create a record of this scan using your own model's representation */
    func add(_ scene: ServerScene)
    
    /** Update your own model's representation using the given scan */
    func upsert(_ scene: ServerScene)
    
    /** Called when a GLTF file finishes downloading to the given local file URL */
//    func didDownloadGLTFFile(for scene: ServerScene, to url: URL)
    
    /** Delete the your own model's representation of the given scan */
    func delete(_ scene: ServerScene)
    
    /** Return the full local file URL of where the scan's GLTF file should be
        expected on disk. If the file does not exist yet, return a URL of where
        it should be downloaded to. */
    func localGLTFURL(for scene: ServerScene) -> URL?
    
    /** Return the full local file URL of where the scenes's thumbnail file should
        be expected on disk. If the file does not exist yet, return a URL of where
        it should be downloaded to. */
    func localThumbnailURL(for scene: ServerScene) -> URL?
}

/** Represents the data necessary to upload a new scene to the server. */
public struct LocalSceneFile {
    public enum Path: Equatable {
        case empty
        case stored(gltfURL: URL, thumbnailURL: URL?)
    }

    /// Unique identifier for this particular scene file. Won't be synced to the server.
    public let localId: String
    public let path: Path
    public let teamKey: String
    public let serverKey: String?
    
    public var gltfURL: URL? {
        switch path {
        case .stored(let gltfURL, _): return gltfURL
        case .empty: return nil
        }
    }
    
    public var thumbnailURL: URL? {
        switch path {
        case .stored(_, let thumbnailURL): return thumbnailURL
        case .empty: return nil
        }
    }
    
    public init(localId: String, path: Path, teamKey: String, serverKey: String?) {
        self.localId = localId
        self.path = path
        self.teamKey = teamKey
        self.serverKey = serverKey
    }
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
        
        ServerFetchScenesOperation(dataSource: _dataSource, serverAPIClient: _serverAPIClient)
        .perform { result in
            switch result {
            case .success(let scenes):
                var sceneFilesToUpload: [LocalSceneFile] = []
                try? self._reconcileRemoteScenesWithLocal(scenes, scenesToUpload: &sceneFilesToUpload)

                self._upload(sceneFilesToUpload) { error in
                    self.isSyncing = false
                    completion(error)
                }

            case .failure(let error):
                completion(error as? ServerOperationError)
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
    public func uploadStatus(for scene: ServerScene) -> UploadStatus {
        if let uploadStatus = _uploadStatusPerSceneUUID[scene.key] {
            return uploadStatus
        } else if let status = scene.uploadStatus {
            return status
        } else {
            return .notUploaded
        }
    }
    
    // MARK: - Private
    
    private var _uploadStatusPerSceneUUID: [String: UploadStatus] = [:]
    
    private func _setSyncStatus(_ status: UploadStatus, for scan: ServerScan) {
        DispatchQueue.main.async {
            NotificationCenter.default.post(name: ServerSyncEngine.syncDidStartNotification, object: self)
        }
    }
    
    private func _reconcileRemoteScenesWithLocal(_ remoteScenes: [ServerScene], scenesToUpload: inout [LocalSceneFile]) throws {
        let localSceneFiles = _dataSource.localSceneFiles().filter { $0.path != .empty }
        
        let remoteScenesByServerKey: [String : ServerScene] = remoteScenes.reduce(into: [:]) { (result, scene) in result[scene.key] = scene }
        
        // Scenes available locally but not remotely should be uploaded
        //  -> scene has a documentsRelativeGLTFPath, but no uid
        // Scenes available both locally and remotely should be updated from the server
        // Deleting locally nor remotely should not happen during a sync
        var unvisitedServerKeys = Set(remoteScenesByServerKey.keys)
        
        for localSceneFile in localSceneFiles {
            guard let key = localSceneFile.serverKey else {
                // Available locally, but not remotely
                scenesToUpload.append(localSceneFile)
                continue
            }
            
            unvisitedServerKeys.remove(key)
            
            // if let remoteScene = remoteScenesByServerKey[key] {
            //     Available both locally and locally, but there's nothing to update
            // }
        }
        
        _dataSource.beginWriteTransaction()
        
        for key in unvisitedServerKeys {
            // Available remotely but not locally
            let scene = remoteScenesByServerKey[key]!
            _dataSource.add(scene)
        }
        
        _dataSource.endWriteTransaction()
    }
    
    private func _upload(_ sceneFilesToUpload: [LocalSceneFile], completion: @escaping (ServerOperationError?) -> Void) {
        let uploadPromises: [Promise<Void>] = sceneFilesToUpload
            .compactMap { $0.gltfURL != nil ? $0 : nil }
            .map {
                (sceneFile: $0,
                 operation: ServerAddSceneOperation(
                    gltfURL: $0.gltfURL!,
                    thumbnailURL: $0.thumbnailURL,
                    teamKey: $0.teamKey,
                    metadata: [:],
                    dataSource: self._dataSource,
                    serverAPIClient: self._serverAPIClient)
                )
        }.map { (sceneFile, operation) in
            Promise<Void> { seal in
                print("Uploading scan from \(operation.gltfURL.path)")
                
                let gltfURL = operation.gltfURL
                self._uploadStatusPerSceneUUID[sceneFile.localId] = .uploading(percentComplete: 0)
                
                operation.perform(uploadProgress: { percentComplete in
                    self._uploadStatusPerSceneUUID[sceneFile.localId] = .uploading(percentComplete: percentComplete)
                }, completion: { result in
                    self._uploadStatusPerSceneUUID[sceneFile.localId] = .uploaded
                    self.syncingCount -= 1
                    
                    switch result {
                    case .success:
                        print("Uploaded scan from \(gltfURL.path)")
                        seal.resolve(nil)
                        
                    case .failure(let error):
                        print("Error uploading scan from \(gltfURL.path): \(error)")
                        seal.resolve(error)
                    }
                })
            }
        }
                
        print("Uploading \(uploadPromises.count) scenes")
        let allPromises = uploadPromises
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
