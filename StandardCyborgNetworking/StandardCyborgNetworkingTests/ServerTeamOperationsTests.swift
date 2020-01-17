//  ServerTeamOperationsTests.swift
//
//  Created by Aaron Thompson on 12/27/19.
//

import StandardCyborgNetworking
import XCTest

class ServerTeamOperationsTests: XCTestCase {
    
    let apiClient = TestAPIClient()
    let dataSource = TestDataSource()
    
    func testServerCreateTeamOperation() {
        let expect = expectation(description: "ServerCreateTeamOperation")
        
        apiClient.setResponse(for: "teams", method: .POST, jsonPath: _fullTestFixturePath("teams-post-success.json"))
        
        ServerCreateTeamOperation(dataSource: dataSource, apiClient: apiClient, name: "ACME Inc", agreesToTermsOfService: true)
        .perform { result in
            switch result {
            case let .success(team):
                XCTAssertEqual(team.name, "ACME Inc")
                XCTAssertEqual(team.uid, "NrBQYaNm")
            case let .failure(error):
                XCTFail("Failure: \(error)")
            }
            
            expect.fulfill()
        }
        
        wait(for: [expect], timeout: 5)
    }
    
    // Fetching all teams is now part of the /me action via ServerGetSignedInUserInfoOperation
    
    private func _fullTestFixturePath(_ relativePath: String) -> String {
        let baseUrl = Bundle(for: TestDataSource.self).resourceURL!
        return baseUrl.appendingPathComponent(relativePath).path
    }
}
