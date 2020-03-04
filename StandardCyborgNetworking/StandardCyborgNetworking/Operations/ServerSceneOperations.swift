//
//  ServerSceneOperations.swift
//  StandardCyborgNetworking
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import PromiseKit


private struct ClientAPIPath {
    static let scenes = "scenes"
    static let teamScenes = "teams/:uid/scenes"
    static let scenesVersions = "scenes/:uid/versions/:version_number"
    static let scenesAssets = "scenes/:uid/assets"
}

public class ServerFetchScenesOperation: ServerOperation {
    public func perform(_ completion: @escaping (Result<[ServerScene]>) -> Void) {
        let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.scenes)
        serverAPIClient.performJSONOperation(withURL: url,
                                             httpMethod: .GET,
                                             httpBodyDict: nil,
                                             responseObjectRootKey: "scenes",
                                             completion: completion)
    }
}

public class ServerAddSceneOperation: ServerOperation {
    let gltfURL: URL
    let thumbnailURL: URL?
    let teamKey: String?
    let metadata: [String: Any]
    
    public init(gltfURL: URL,
                thumbnailURL: URL? = nil,
                teamKey: String? = nil,
                metadata: [String: Any] = [:],
                dataSource: ServerSyncEngineLocalDataSource,
                serverAPIClient: ServerAPIClient)
    {
        self.gltfURL = gltfURL
        self.thumbnailURL = thumbnailURL
        self.teamKey = teamKey
        self.metadata = metadata
        
        for value in metadata.values {
            assert(value is Bool || value is Int || value is Float || value is Double || value is String)
        }
        
        super.init(dataSource: dataSource, serverAPIClient: serverAPIClient)
    }
    
    public func perform(uploadProgress: ((Double) -> Void)?,
                        completion: @escaping (Result<ServerScene>) -> Void)
    {
        var scene: ServerScene!
        var sceneUploadInfo: S3UploadInfo!
        var thumbnailUploadInfo: S3UploadInfo?
        
        var promise =
        firstly {
            // 1. Submit an empty scene to /scenes
            self._createEmptyScene()
            // 2. Receive a response with a scene `uid` and an initial `version_number`
            // 3. Associate the files on the device with the returned scene `uid` and `version_number`
        }.map { emptyScene in
            scene = emptyScene
        }.then {
            // 4. Upload files to S3 via the POST `/direct_uploads` endpoint
            self._createS3FileReference(for: self.gltfURL, remoteFilename: "Scene.gltf")
        }.then { localURL, uploadInfo in
            self._uploadFileToS3(localURL: localURL, uploadInfo: uploadInfo, progressHandler: uploadProgress)
        }.map { uploadInfo in
            sceneUploadInfo = uploadInfo
        }
        
        if let thumbnailURL = thumbnailURL { promise = promise
            .then {
                return self._createS3FileReference(for: thumbnailURL, remoteFilename: "thumbnail.jpeg")
            }.then { localURL, uploadInfo in
                self._uploadFileToS3(localURL: localURL, uploadInfo: uploadInfo, progressHandler: nil)
            }.map { uploadInfo in
                thumbnailUploadInfo = uploadInfo
            }
        }
        
        promise = promise.then { _ in
            // 5. Update the version created in step 2 to fill in the pending scenegraph_key and thumbnail
            self._updateScene(scene, sceneUploadInfo: sceneUploadInfo, thumbnailUploadInfo: thumbnailUploadInfo)
        }.map { updatedScene in
            scene = updatedScene
        }
        
        promise.done {
            completion(.success(scene))
        }.catch { error in
            let serverError = error as? ServerOperationError ?? ServerOperationError.genericError(error)
            completion(.failure(serverError))
        }
    }
    
    private func _createEmptyScene() -> Promise<ServerScene> {
        return Promise { seal in
            let url: URL
            if let teamKey = teamKey {
                var tempURLString = serverAPIClient.buildAPIURL(for: ClientAPIPath.teamScenes).absoluteString
                tempURLString = tempURLString.replacingOccurrences(of: ":uid", with: teamKey)
                url = URL(string: tempURLString)!
            } else {
                url = serverAPIClient.buildAPIURL(for: ClientAPIPath.scenes)
            }
            
            serverAPIClient.performJSONOperation(withURL: url,
                                                 httpMethod: HTTPMethod.POST,
                                                 httpBodyDict: nil,
                                                 responseObjectRootKey: "scene")
            { (result: Result<ServerScene>) in
                switch result {
                case .success(let scene):
                    print("Successfully created server scene with uid \(scene.key ?? "unknown")")
                    seal.fulfill(scene)
                    
                case .failure(let error):
                    print("Failed to create empty scene on server")
                    seal.reject(error)
                }
            }
        }
    }
    
    private func _uploadFileToS3(localURL: URL, uploadInfo: S3UploadInfo, progressHandler: ((Double) -> Void)?) -> Promise<S3UploadInfo> {
        return Promise<S3UploadInfo> { seal in
            print("Uploading file to S3 for \(uploadInfo.directUploadFileKey) from local path \(localURL.path)")
            serverAPIClient.performDataUploadOperation(withURL: uploadInfo.url,
                                                       httpMethod: HTTPMethod.PUT,
                                                       dataURL: localURL,
                                                       extraHeaders: uploadInfo.uploadHeaders,
                                                       progressHandler: progressHandler)
            { error in
                guard error == nil else { return seal.reject(error!) }
                
                print("Successfully uploaded scene file to S3")
                seal.fulfill(uploadInfo)
            }
        }
    }
    
    private func _updateScene(_ scene: ServerScene, sceneUploadInfo: S3UploadInfo, thumbnailUploadInfo: S3UploadInfo?) -> Promise<ServerScene> {
        return Promise { seal in
            guard let sceneKey = scene.key else {
                seal.reject(ServerOperationError.genericErrorString("This ServerScene has no server key: \(scene)"))
                return
            }
            guard let sceneVersion = scene.versions.first else {
                seal.reject(ServerOperationError.genericErrorString("This ServerScene has no versions: \(scene)"))
                return
            }
            
            var tempURLString = serverAPIClient.buildAPIURL(for: ClientAPIPath.scenesVersions).absoluteString
            tempURLString = tempURLString.replacingOccurrences(of: ":uid", with: sceneKey)
            tempURLString = tempURLString.replacingOccurrences(of: ":version_number", with: "\(sceneVersion.versionNumber)")
            let url = URL(string: tempURLString)!
            
            // Convert the scene object to a dictionary by encoding it ServerScene => Data => Dictionary.
            // We then embed that dictionary inside a root "scene" object since that's what the server expects.
            var sceneVersionDict: [String: Any] = ["scenegraph_key": sceneUploadInfo.directUploadFileKey]
            sceneVersionDict["thumbnail_key"] = thumbnailUploadInfo?.directUploadFileKey
            sceneVersionDict["metadata"] = metadata
            
            serverAPIClient.performJSONOperation(withURL: url,
                                                 httpMethod: .PUT,
                                                 httpBodyDict: ["scene_version" : sceneVersionDict],
                                                 responseObjectRootKey: "scene_version")
            { (result: Result<ServerSceneVersion>) in
                switch result {
                case .success(var updatedSceneVersion):
                    print("Successfully updated scene with uid \(updatedSceneVersion.key ?? "unknown")")
                    // self.dataSource.update(updatedSceneVersion)
                    // Take the local UUID from the one this is replacing
                    updatedSceneVersion.localUUID = sceneVersion.localUUID
                    updatedSceneVersion.uploadStatus = .uploaded
                    
                    var updatedScene = scene
                    updatedScene.versions = [updatedSceneVersion]
                    seal.fulfill(updatedScene)

                case .failure(let error):
                    print("Failed to get scene info from POST to \(url)")
                    seal.reject(error)
                }
            }
        }
    }
}

public class ServerAddSceneAssetOperation: ServerOperation {
    let fileAssetURL: URL
    let teamKey: String?
    let sceneKey: String?
    let remoteFileName: String?
    
    public init(fileAssetURL: URL,
                teamKey: String? = nil,
                sceneKey: String? = nil,
                remoteFileName: String,
                dataSource: ServerSyncEngineLocalDataSource,
                serverAPIClient: ServerAPIClient)
    {
        self.fileAssetURL = fileAssetURL
        self.teamKey = teamKey
        self.sceneKey = sceneKey
        self.remoteFileName = remoteFileName
        
        super.init(dataSource: dataSource, serverAPIClient: serverAPIClient)
    }

    public func perform(uploadProgress: ((Double) -> Void)?,
                        completion: @escaping (Result<ServerSceneAsset>?) -> Void)
    {
        // fetch the scene id
        var sceneAsset: ServerSceneAsset!
        var sceneAssetUploadInfo: S3UploadInfo!
        
        //1. fetch scene by key to make sure it's valid
        //2. create direct upload
        //3. upload to direct upload
        //4. post to scenes/:uid/assets with direct upload key and scene uid to link as asset
        
        var promise =
        firstly {
            self._createS3FileReference(for: self.fileAssetURL, remoteFilename: (self.remoteFileName!))
        }.then { localURL, uploadInfo in
            self._uploadFileToS3(localURL: localURL, uploadInfo: uploadInfo, progressHandler: uploadProgress)
        }.map { uploadInfo in
            sceneAssetUploadInfo = uploadInfo
        }
        
        promise = promise.then { _ in
            self._updateScene(self.sceneKey!, sceneUploadInfo: sceneAssetUploadInfo)
        }.map { newSceneAsset in
            sceneAsset = newSceneAsset
        }
        
        promise.done {
            completion(.success(sceneAsset))
        }.catch { error in
            let serverError = error as? ServerOperationError ?? ServerOperationError.genericError(error)
            completion(.failure(serverError))
        }

    }
    
    private func _updateScene(_ sceneKey: String, sceneUploadInfo: S3UploadInfo) -> Promise<ServerSceneAsset> {
        return Promise { seal in

            var tempURLString = serverAPIClient.buildAPIURL(for: ClientAPIPath.scenesAssets).absoluteString
            tempURLString = tempURLString.replacingOccurrences(of: ":uid", with: sceneKey)
            let url = URL(string: tempURLString)!

            serverAPIClient.performJSONOperation(withURL: url,
                                                 httpMethod: .POST,
                                                 httpBodyDict: ["scene_asset" : ["asset_key": sceneUploadInfo.directUploadFileKey]],
                                                 responseObjectRootKey: "scene_asset")
            { (result: Result<ServerSceneAsset>) in
                print(result)
                switch result {
                case .success(let sceneAsset):
                    print("Successfully created scene asset with for scene with uid \(sceneAsset.scene_uid ?? "unknown")")
                    seal.fulfill(sceneAsset)
                    
                case .failure(let error):
                    print("Failed to get scene info from POST to \(url)")
                    seal.reject(error)
                }
            }
        }
    }
    
    private func _uploadFileToS3(localURL: URL, uploadInfo: S3UploadInfo, progressHandler: ((Double) -> Void)?) -> Promise<S3UploadInfo> {
        return Promise<S3UploadInfo> { seal in
            print("Uploading file to S3 for \(uploadInfo.directUploadFileKey) from local path \(localURL.path)")
            serverAPIClient.performDataUploadOperation(withURL: uploadInfo.url,
                                                       httpMethod: HTTPMethod.PUT,
                                                       dataURL: localURL,
                                                       extraHeaders: uploadInfo.uploadHeaders,
                                                       progressHandler: progressHandler)
            { error in
                guard error == nil else { return seal.reject(error!) }
                
                print("Successfully uploaded scene file to S3")
                seal.fulfill(uploadInfo)
            }
        }
    }

}

public class ServerDeleteSceneOperation: ServerOperation {
    let scene: ServerScene
    
    public init(scene: ServerScene,
                dataSource: ServerSyncEngineLocalDataSource,
                serverAPIClient: ServerAPIClient)
    {
        self.scene = scene
        super.init(dataSource: dataSource, serverAPIClient: serverAPIClient)
    }
    
    public func perform(_ completion: @escaping (ServerOperationError?) -> Void)
    {
        guard let key = scene.key else {
            completion(ServerOperationError.genericErrorString("Could not delete scene with no server key"))
            return
        }
        
        let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.scenes)
                .appendingPathComponent(key)
        
        serverAPIClient.performJSONOperation(withURL: url,
                                             httpMethod: .DELETE,
                                             httpBodyDict: nil,
                                             responseObjectRootKey: nil)
        { (result: Result<SuccessResponse>) in
            switch result {
            case .success(_):
                completion(nil)
            case .failure(let error):
                completion(error as? ServerOperationError)
            }
        }
    }
    
}
