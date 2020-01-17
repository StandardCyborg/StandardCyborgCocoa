//
//  ServerTeam.swift
//  StandardCyborgNetworking
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

import Foundation

public struct ServerTeam: Codable {
    public enum Role: String, Codable {
        case `default`
        case admin
    }
    
    public let name: String
    public let uid: String
    public let role: Role
}
