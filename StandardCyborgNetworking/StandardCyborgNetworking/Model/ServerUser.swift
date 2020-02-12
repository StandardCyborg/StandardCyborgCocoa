//
//  ServerUser.swift
//  StandardCyborgNetworking
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

import Foundation

public struct ServerUser: Codable {
    
    public var key: String?
    public var email: String?
    public var name: String?
    public var teams: [ServerTeam]?
    
    enum CodingKeys: String, CodingKey {
        case key = "uid", email, name, teams
    }
    
    public init(key: String? = nil, email: String? = nil, name: String? = nil, teams: [ServerTeam]? = nil) {
        self.key = key
        self.email = email
        self.name = name
        self.teams = teams
    }
}
