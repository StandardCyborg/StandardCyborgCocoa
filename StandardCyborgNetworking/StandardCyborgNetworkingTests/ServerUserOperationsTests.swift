//  StandardCyborgNetworkingTests.swift
//
//  Created by Aaron Thompson on 12/27/19.
//

import StandardCyborgNetworking
import XCTest

class ServerUserOperationsTests: XCTestCase {
    
    let apiClient = TestAPIClient()
    let dataSource = TestDataSource()
    
    // override func setUp() {}
    
    func testServerSignUpOperation() {
        let expect = expectation(description: "ServerSignUpOperation")
        
        apiClient.responseJSONPath = _fullTestFixturePath("auth-sign_up-success.json")
        
        ServerSignUpOperation(dataSource: dataSource, apiClient: apiClient, email: "test@example.com", password: "password")
        .perform { result in
            switch result {
            case let .success(user):
                XCTAssertEqual(user.email, "test@example.com")
                XCTAssertEqual(user.name, "John Doe")
                XCTAssertEqual(user.key, user.email)
                XCTAssertEqual(user.teams?.first?.name, "John Doe")
                XCTAssertEqual(user.teams?.first?.personal, true)
                XCTAssertEqual(user.teams?.first?.uid, "uMQrVFHx")
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
        
        apiClient.responseJSONPath = _fullTestFixturePath("auth-sign_in-success.json")
        
        ServerSignInOperation(dataSource: dataSource, apiClient: apiClient, email: "test@example.com", password: "password")
        .perform { result in
            switch result {
            case let .success(user):
                XCTAssertEqual(user.email, "test@example.com")
                XCTAssertEqual(user.name, "John Doe")
                XCTAssertEqual(user.key, "siuayACV")
                XCTAssertEqual(user.teams?.first?.name, "John Doe")
                XCTAssertEqual(user.teams?.first?.personal, true)
                XCTAssertEqual(user.teams?.first?.uid, "uMQrVFHx")
            case let .failure(error):
                XCTFail("Failure: \(error)")
            }
            successExpect.fulfill()
        }
        wait(for: [successExpect], timeout: 5)
        
        apiClient.responseJSONPath = _fullTestFixturePath("auth-sign_in-failure.json")
        
        ServerSignInOperation(dataSource: dataSource, apiClient: apiClient, email: "test@example.com", password: "password")
        .perform { result in
            switch result {
            case let .success(user):
                XCTFail("Expected failure but got user \(user)")
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
