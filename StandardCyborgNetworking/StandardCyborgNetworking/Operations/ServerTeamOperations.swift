//
//  ServerUserOperations.swift
//  StandardCyborgNetworking
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import class PromiseKit.Promise

private struct ClientAPIPath {
    static let teams = "teams"
}

public class ServerCreateTeamOperation: ServerOperation {
    
    let name: String
    let agreesToTermsOfService: Bool
    
    public init(dataSource: ServerSyncEngineLocalDataSource,
                apiClient: ServerAPIClient,
                name: String,
                agreesToTermsOfService: Bool)
    {
        self.name = name
        self.agreesToTermsOfService = agreesToTermsOfService
        super.init(dataSource: dataSource, serverAPIClient: apiClient)
    }
    
    public func perform(_ completion: @escaping (Result<ServerTeam>) -> Void) {
        let postDictionary: [String:Any] = [
            "name": name,
            "agrees_to_terms_of_service": agreesToTermsOfService
        ]
        let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.teams)
        
        serverAPIClient.performJSONOperation(withURL: url,
                                             httpMethod: .POST,
                                             httpBodyDict: postDictionary,
                                             responseObjectRootKey: "team")
        { (result: Result<ServerTeam>) in
            completion(result)
        }
    }
    
}

public class ServerFetchTeamsOperation: ServerOperation {
    
    public func perform(_ completion: @escaping (Result<[ServerTeam]>) -> Void) {
        let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.teams)
        
        serverAPIClient.performJSONOperation(withURL: url,
                                             httpMethod: .POST,
                                             httpBodyDict: nil,
                                             responseObjectRootKey: "teams")
        { (result: Result<[ServerTeam]>) in
            completion(result)
        }
    }
    
}
