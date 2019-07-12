//
//  ServerTeam.swift
//  StandardCyborgNetworking
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//
import Foundation

public struct ServerAccessToken: Codable {

    public let accessToken: String
    public let apiKey: String

    private enum CodingKeys: String, CodingKey {
        case accessToken = "Access-Token"
        case apiKey = "Api-Key"
    }
}
