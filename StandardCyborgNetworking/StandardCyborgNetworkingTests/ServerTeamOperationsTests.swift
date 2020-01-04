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
    
    func testServerFetchTeamsOperation() {
        let expect = expectation(description: "ServerFetchTeamsOperation")
        
        apiClient.setResponse(for: "teams", method: .POST, jsonPath: _fullTestFixturePath("teams-get-success.json"))
        
        ServerFetchTeamsOperation(dataSource: dataSource, serverAPIClient: apiClient)
        .perform { result in
            switch result {
            case let .success(teams):
                XCTAssertEqual(teams.count, 2)
                XCTAssertEqual(teams.first?.name, "Team 1")
                XCTAssertEqual(teams.first?.uid, "vMU9e67M")
                XCTAssertEqual(teams.last?.name, "Team 2")
                XCTAssertEqual(teams.last?.uid, "qWFVgRPY")
            case let .failure(error):
                XCTFail("Failure: \(error)")
            }
            
            expect.fulfill()
        }
        
        wait(for: [expect], timeout: 5)
    }
    
    private func _fullTestFixturePath(_ relativePath: String) -> String {
        let baseUrl = Bundle(for: TestDataSource.self).resourceURL!
        return baseUrl.appendingPathComponent(relativePath).path
    }
}
