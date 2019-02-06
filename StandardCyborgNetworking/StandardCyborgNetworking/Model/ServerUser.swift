//
//  ServerUser.swift
//  StandardCyborgNetworking
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

import Foundation

public struct ServerUser {
    
    public var key: String?
    public var team: String?
    public var email: String?
    public var name: String?
    
    public init(key: String? = nil, team: String? = nil, email: String? = nil, name: String? = nil) {
        self.key = key
        self.team = team
        self.email = email
        self.name = name
    }
    
}
