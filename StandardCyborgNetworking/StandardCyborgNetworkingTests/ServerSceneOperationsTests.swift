//  ServerSceneOperationsTests.swift
//
//  Created by Aaron Thompson on 12/27/19.
//

import StandardCyborgNetworking
import XCTest

class ServerSceneOperationsTests: XCTestCase {
    
    let apiClient = TestAPIClient()
    let dataSource = TestDataSource()
    
    // override func setUp() {}
    
    func testServerAddSceneOperation() {
        let expect = expectation(description: "ServerAddSceneOperation")
        
        apiClient.setResponse(for: "scenes", jsonPath: _fullTestFixturePath("scenes-success.json"))
        apiClient.setResponse(for: "scenes/8UHPFfY7/versions/1", jsonPath: _fullTestFixturePath("scenes-uid-versions-success.json"))
        apiClient.setResponse(for: "direct_uploads", jsonPath: _fullTestFixturePath("direct_uploads-success.json"))
        
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
    
    func testServerSignInOperation() {
        let successExpect = expectation(description: "ServerSignInOperation-success")
        let failureExpect = expectation(description: "ServerSignInOperation-failure")
        
        apiClient.setResponse(for: "auth/sign_in", jsonPath: _fullTestFixturePath("auth-sign_in-success.json"))
        
        ServerSignInOperation(dataSource: dataSource, apiClient: apiClient, email: "test@example.com", password: "password")
        .perform { result in
            switch result {
            case let .success(scene):
                XCTAssertEqual(scene.email, "test@example.com")
                XCTAssertEqual(scene.name, "John Doe")
                XCTAssertEqual(scene.key, "siuayACV")
                XCTAssertEqual(scene.teams?.first?.name, "John Doe")
                XCTAssertEqual(scene.teams?.first?.personal, true)
                XCTAssertEqual(scene.teams?.first?.uid, "uMQrVFHx")
            case let .failure(error):
                XCTFail("Failure: \(error)")
            }
            successExpect.fulfill()
        }
        wait(for: [successExpect], timeout: 5)
        
        apiClient.setResponse(for: "auth/sign_in", jsonPath: _fullTestFixturePath("auth-sign_in-failure.json"))
        
        ServerSignInOperation(dataSource: dataSource, apiClient: apiClient, email: "test@example.com", password: "password")
        .perform { result in
            switch result {
            case let .success(scene):
                XCTFail("Expected failure but got scene \(scene)")
            case let .failure(error):
                XCTAssertTrue(error is ServerOperationError)
                XCTAssertEqual(error as! ServerOperationError, ServerOperationError.genericErrorString("Invalid login credentials. Please try again."))
            }
            failureExpect.fulfill()
        }
        wait(for: [failureExpect], timeout: 5)
    }
    
    private func _fullTestFixturePath(_ relativePath: String) -> String {
        let baseUrl = Bundle(for: TestDataSource.self).resourceURL!
        return baseUrl.appendingPathComponent(relativePath).path
    }
}
