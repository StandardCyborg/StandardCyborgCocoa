//  ServerSceneOperationsTests.swift
//
//  Created by Aaron Thompson on 12/27/19.
//

import StandardCyborgNetworking
import XCTest

class ServerSceneOperationsTests: XCTestCase {
    
    let apiClient = TestAPIClient()
    let dataSource = TestDataSource()
    
    func testServerAddSceneOperation() {
        let expect = expectation(description: "ServerAddSceneOperation")
        
        apiClient.setResponse(for: "scenes", method: .POST, jsonPath: _fullTestFixturePath("scenes-post-success.json"))
        apiClient.setResponse(for: "scenes/8UHPFfY7/versions/1", method: .PUT, jsonPath: _fullTestFixturePath("scenes-uid-versions-put-success.json"))
        apiClient.setResponse(for: "direct_uploads", method: .POST, jsonPath: _fullTestFixturePath("direct_uploads-post-success.json"))
        
        let gltfPath = _fullTestFixturePath("empty.gltf")
        let thumbnailPath = _fullTestFixturePath("empty.jpeg")
        
        ServerAddSceneOperation(gltfURL: URL(fileURLWithPath: gltfPath),
                                thumbnailURL: URL(fileURLWithPath: thumbnailPath),
                                dataSource: dataSource,
                                serverAPIClient: apiClient)
        .perform(uploadProgress: nil) { result in
            switch result {
            case let .success(scene):
                XCTAssertEqual(scene.createdAt, DateTimeTransform.fromString("2019-11-20T21:13:24.607Z"))
                XCTAssertEqual(scene.key, "8UHPFfY7")
                XCTAssertNil(scene.teamUID)
                XCTAssertEqual(scene.versions.first?.versionNumber, 3)
                XCTAssertEqual(scene.versions.first?.key, "tSGUGGfp")
            case let .failure(error):
                XCTFail("Failure: \(error)")
            }
            
            expect.fulfill()
        }
        
        wait(for: [expect], timeout: 5)
    }
    
    func testServerFetchScenesOperation() {
        let expect = expectation(description: "ServerFetchScenesOperation")
        
        apiClient.setResponse(for: "scenes", method: .GET, jsonPath: _fullTestFixturePath("scenes-get-success.json"))
        
        ServerFetchScenesOperation(dataSource: dataSource,
                                   serverAPIClient: apiClient)
        .perform { result in
            switch result {
            case let .success(scenes):
                XCTAssertEqual(scenes.count, 2)
                
                XCTAssertEqual(scenes.first?.createdAt, DateTimeTransform.fromString("2019-12-29T15:41:29.783Z"))
                XCTAssertEqual(scenes.first?.key, "bGaWFVR7")
                XCTAssertEqual(scenes.first?.teamUID, "mWD982Bo")
                
                XCTAssertEqual(scenes.first?.versions.count, 1)
                XCTAssertEqual(scenes.first?.versions.first?.versionNumber, 1)
                XCTAssertEqual(scenes.first?.versions.first?.key, "UWcv2KmW")
                XCTAssertEqual(scenes.first?.versions.first?.sceneUID, "bGaWFVR7")
            case let .failure(error):
                XCTFail("Failure: \(error)")
            }
            
            expect.fulfill()
        }
        
        wait(for: [expect], timeout: 5)
    }
    
    func testServerDeleteSceneOperation() {
        let expect = expectation(description: "ServerFetchScenesOperation")
        
        apiClient.setResponse(for: "scenes/abc123", method: .DELETE, jsonPath: _fullTestFixturePath("scenes-uid-delete-success.json"))
        
        let scene = ServerScene(createdAt: Date(),
                                key: "abc123",
                                teamUID: "def456",
                                versions: [])
        
        ServerDeleteSceneOperation(scene: scene,
                                   dataSource: dataSource,
                                   serverAPIClient: apiClient)
        .perform { error in
            XCTAssertNil(error)
            expect.fulfill()
        }
        
        wait(for: [expect], timeout: 5)
    }
    
    private func _fullTestFixturePath(_ relativePath: String) -> String {
        let baseUrl = Bundle(for: TestDataSource.self).resourceURL!
        return baseUrl.appendingPathComponent(relativePath).path
    }
}
