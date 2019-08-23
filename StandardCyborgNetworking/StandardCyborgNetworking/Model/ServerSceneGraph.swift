//
//  ServerSceneGraph.swift
//  StandardCyborgNetworking
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

import Foundation

public struct ServerSceneGraph: Codable {
    
    private enum CodingKeys: String, CodingKey {
        case createdAt
        case key = "uid"
        case teamUID = "team_uid"
        case sceneVersions
    }
    
    public var localUUID = UUID()
    
    public let createdAt: Date?
    public let key: String?
    public let teamUID: String?
    public var versions: [ServerSceneVersion]
    
    // MARK: - Codable
    
    public init(from decoder: Decoder) {
        let container = try! decoder.container(keyedBy: CodingKeys.self)
        
        createdAt = try! container.decodeDateStringIfPresent(forKey: .createdAt)
        key = try! container.decodeIfPresent(String.self, forKey: .key)
        teamUID = try! container.decodeIfPresent(String.self, forKey: .teamUID)
        versions = try! container.decodeIfPresent([ServerSceneVersion].self, forKey: .sceneVersions) ?? []
    }
    
    public func encode(to encoder: Encoder) {
        var container = encoder.container(keyedBy: CodingKeys.self)
        
        try? container.encodeDateAsStringIfPresent(createdAt, forKey: .createdAt)
        try? container.encodeIfPresent(key, forKey: .key)
        try? container.encodeIfPresent(teamUID, forKey: .teamUID)
        try? container.encodeIfPresent(versions, forKey: .sceneVersions)
    }
}


public struct ServerSceneVersion: Codable {
    
    private enum CodingKeys: String, CodingKey {
        case createdAt
        case versionNumber
        case parentVersionNumber
        case key = "uid"
        case sceneUid
        case scenegraphUrl
        case thumbnailUrl
    }
    
    public var localUUID = UUID()
    public var uploadStatus: UploadStatus?
    
    public var createdAt: Date?
    public let versionNumber: Int
    public let parentVersionNumber: Int
    public let key: String?
    public var sceneUID: String?
    public var sceneGraphURL: URL?
    
    // MARK: - Codable
    
    public init(from decoder: Decoder) {
        let container = try! decoder.container(keyedBy: CodingKeys.self)
        
        createdAt = try! container.decodeDateStringIfPresent(forKey: CodingKeys.createdAt)
        versionNumber = try! container.decode(Int.self, forKey: CodingKeys.versionNumber)
        parentVersionNumber = try! container.decode(Int.self, forKey: CodingKeys.parentVersionNumber)
        key = try! container.decodeIfPresent(String.self, forKey: .key)
        sceneUID = try! container.decode(String.self, forKey: CodingKeys.sceneUid)
        sceneGraphURL = try! container.decodeIfPresent(URL.self, forKey: CodingKeys.scenegraphUrl)
    }
    
    public func encode(to encoder: Encoder) {
        var container = encoder.container(keyedBy: CodingKeys.self)
        
        try? container.encodeDateAsStringIfPresent(createdAt, forKey: CodingKeys.createdAt)
        try? container.encode(versionNumber, forKey: .versionNumber)
        try? container.encode(parentVersionNumber, forKey: .parentVersionNumber)
        try? container.encodeIfPresent(key, forKey: .key)
        try? container.encodeIfPresent(sceneUID, forKey: .sceneUid)
        try? container.encodeIfPresent(sceneGraphURL, forKey: .scenegraphUrl)
    }
}

private extension KeyedDecodingContainer where Key : CodingKey {
    func decodeDateStringIfPresent(forKey key: Key) throws -> Date? {
        if let dateString = try! decodeIfPresent(String.self, forKey: key) {
            return DateTimeTransform.fromString(dateString)
        }
        return nil
    }
}

private extension KeyedEncodingContainer where Key : CodingKey {
    mutating func encodeDateAsStringIfPresent(_ date: Date?, forKey key: Key) throws {
        if let date = date {
            try? encodeIfPresent(DateTimeTransform.toString(date), forKey: key)
        }
    }
}
