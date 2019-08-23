//
//  ServerSceneGraphOperations.swift
//  StandardCyborgNetworking
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import PromiseKit


private struct ClientAPIPath {
    static let scenes = "scenes"
    static let scenesVersions = "scenes/:uid/versions/:version_number"
}

public class ServerAddSceneGraphOperation: ServerOperation {
    
    let gltfURL: URL
    
    public init(gltfURL: URL,
                dataSource: ServerSyncEngineLocalDataSource,
                serverAPIClient: ServerAPIClient)
    {
        self.gltfURL = gltfURL
        super.init(dataSource: dataSource, serverAPIClient: serverAPIClient)
    }
    
    public func perform(uploadProgress: ((Double) -> Void)?,
                        completion: @escaping (ServerOperationError?, ServerSceneGraph?) -> Void)
    {
        var sceneGraph: ServerSceneGraph!
        
        firstly {
            // 1. Submit an empty scene to /scenes
            self._createEmptySceneGraph()
            // 2. Receive a response with a scene `uid` and an initial `version_number`
            // 3. Associate the files on the device with the returned scene `uid` and `version_number`
        }.map { emptySceneGraph in
            sceneGraph = emptySceneGraph
        }.then {
            // 4. Upload files to S3 via the POST `/direct_uploads` endpoint
            return self._createS3FileReference(for: self.gltfURL, remoteFilename: "SceneGraph.gltf")
        }.then { localURL, uploadInfo in
            self._uploadSceneGraphFileToS3(localURL: localURL, uploadInfo: uploadInfo, progressHandler: uploadProgress)
        }.then { uploadInfo in
            // 5. Update the version created in step 2 to fill in the pending scenegraph_key and thumbnail
            self._updateSceneGraph(sceneGraph, uploadInfo: uploadInfo)
        }.done { sceneGraph in
            completion(nil, sceneGraph)
        }.catch { error in
            let serverError = error as? ServerOperationError ?? ServerOperationError.genericError(error)
            completion(serverError, nil)
        }
    }
    
    private func _createEmptySceneGraph() -> Promise<ServerSceneGraph> {
        return Promise { seal in
            let url: URL = serverAPIClient.buildAPIURL(for: ClientAPIPath.scenes)
            
            serverAPIClient.performJSONOperation(withURL: url,
                                                 httpMethod: HTTPMethod.POST,
                                                 httpBodyDict: nil,
                                                 responseObjectRootKey: "scene")
            { (result: Result<ServerSceneGraph>) in
                switch result {
                case .success(let sceneGraph):
                    print("Successfully created server sceneGraph with uid \(sceneGraph.key ?? "unknown")")
                    seal.fulfill(sceneGraph)
                    
                case .failure(let error):
                    print("Failed to create empty scene graph on server")
                    seal.reject(error)
                }
            }
        }
    }
    
    private func _uploadSceneGraphFileToS3(localURL: URL, uploadInfo: S3UploadInfo, progressHandler: ((Double) -> Void)?) -> Promise<S3UploadInfo> {
        return Promise<S3UploadInfo> { seal in
            print("Uploading sceneGraph file to S3 for \(uploadInfo.directUploadFileKey)")
            serverAPIClient.performDataUploadOperation(withURL: uploadInfo.url,
                                                       httpMethod: HTTPMethod.PUT,
                                                       dataURL: localURL,
                                                       extraHeaders: uploadInfo.uploadHeaders,
                                                       progressHandler: progressHandler)
            { error in
                guard error == nil else { return seal.reject(error!) }
                
                print("Successfully uploaded sceneGraph file to S3")
                seal.fulfill(uploadInfo)
            }
        }
    }
    
    private func _updateSceneGraph(_ sceneGraph: ServerSceneGraph, uploadInfo: S3UploadInfo) -> Promise<ServerSceneGraph> {
        return Promise { seal in
            guard let sceneGraphKey = sceneGraph.key else {
                seal.reject(ServerOperationError.genericErrorString("This ServerSceneGraph has no server key: \(sceneGraph)"))
                return
            }
            guard let sceneVersion = sceneGraph.versions.first else {
                seal.reject(ServerOperationError.genericErrorString("This ServerSceneGraph has no versions: \(sceneGraph)"))
                return
            }
            
            var tempURLString = serverAPIClient.buildAPIURL(for: ClientAPIPath.scenesVersions).absoluteString
            tempURLString = tempURLString.replacingOccurrences(of: ":uid", with: sceneGraphKey)
            tempURLString = tempURLString.replacingOccurrences(of: ":version_number", with: "\(sceneVersion.versionNumber)")
            let url = URL(string: tempURLString)!
            
            // Convert the sceneGraph object to a dictionary by encoding it ServerSceneGraph => Data => Dictionary.
            // We then embed that dictionary inside a root "sceneGraph" object since that's what the server expects.
            let encoder = JSONEncoder()
            encoder.keyEncodingStrategy = .convertToSnakeCase
            let sceneVersionDict = ["scenegraph_key": uploadInfo.directUploadFileKey]
            
            serverAPIClient.performJSONOperation(withURL: url,
                                                 httpMethod: .PUT,
                                                 httpBodyDict: ["scene_version" : sceneVersionDict],
                                                 responseObjectRootKey: "scene_version")
            { (result: Result<ServerSceneVersion>) in
                switch result {
                case .success(var updatedSceneVersion):
                    print("Successfully updated scene graph with uid \(updatedSceneVersion.key ?? "unknown")")
                    // self.dataSource.update(updatedSceneVersion)
                    // Take the local UUID from the one this is replacing
                    updatedSceneVersion.localUUID = sceneVersion.localUUID
                    updatedSceneVersion.uploadStatus = .uploaded
                    
                    var updatedSceneGraph = sceneGraph
                    updatedSceneGraph.versions = [updatedSceneVersion]
                    seal.fulfill(updatedSceneGraph)

                case .failure(let error):
                    print("Failed to get sceneGraph info from POST to \(url)")
                    seal.reject(error)
                }
            }
        }
    }
    
}
