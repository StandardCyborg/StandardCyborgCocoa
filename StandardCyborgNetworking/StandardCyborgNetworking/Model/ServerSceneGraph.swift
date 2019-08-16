//
//  ServerSceneGraph.swift
//  StandardCyborgNetworking
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

import Foundation

public struct ServerSceneGraph: Codable {
    
    private enum CodingKeys: String, CodingKey {
        case createdAt = "created_at"
        case key = "uid"
        case teamUID = "team_uid"
        case versions = "scene_versions"
    }
    
    public var localUUID = UUID()
    
    public let createdAt: Date?
    public let key: String?
    public let teamUID: String?
    public let versions: [ServerSceneVersion]
    
    // MARK: - Codable
    
    public init(from decoder: Decoder) {
        let container = try! decoder.container(keyedBy: CodingKeys.self)
        
        createdAt = try! container.decodeIfPresent(Date.self, forKey: .createdAt)
        key = try! container.decodeIfPresent(String.self, forKey: .key)
        teamUID = try! container.decodeIfPresent(String.self, forKey: .teamUID)
        versions = try! container.decodeIfPresent([ServerSceneVersion].self, forKey: .versions) ?? []
    }
    
    public func encode(to encoder: Encoder) {
        var container = encoder.container(keyedBy: CodingKeys.self)
        
        try? container.encodeIfPresent(createdAt, forKey: .createdAt)
        try? container.encodeIfPresent(key, forKey: .key)
        try? container.encodeIfPresent(teamUID, forKey: .teamUID)
        try? container.encodeIfPresent(versions, forKey: .versions)
    }
}


public struct ServerSceneVersion: Codable {
    
    private enum CodingKeys: String, CodingKey {
        case createdAt = "created_at"
        case version = "version_number"
        case parentVersion = "parent_version_number"
        case key = "uid"
        case sceneUID = "scene_uid"
        case sceneGraphURL = "scenegraph_url"
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
        versionNumber = try! container.decode(Int.self, forKey: CodingKeys.version)
        parentVersionNumber = try! container.decode(Int.self, forKey: CodingKeys.parentVersion)
        key = try! container.decodeIfPresent(String.self, forKey: .key)
        sceneUID = try! container.decode(String.self, forKey: CodingKeys.sceneUID)
        sceneGraphURL = try! container.decode(URL.self, forKey: CodingKeys.sceneGraphURL)
    }
    
    public func encode(to encoder: Encoder) {
        var container = encoder.container(keyedBy: CodingKeys.self)
        
        try? container.encodeDateAsStringIfPresent(createdAt, forKey: CodingKeys.createdAt)
        try? container.encode(versionNumber, forKey: .version)
        try? container.encode(parentVersionNumber, forKey: .parentVersion)
        try? container.encodeIfPresent(key, forKey: .key)
        try? container.encodeIfPresent(sceneUID, forKey: .sceneUID)
        try? container.encodeIfPresent(sceneGraphURL, forKey: .sceneGraphURL)
    }
}

private extension KeyedDecodingContainer where Key : CodingKey {
    func decodeDateStringIfPresent(forKey key: Key) throws -> Date? {
        if let dateString = try? decodeIfPresent(String.self, forKey: key) {
            return DateTimeTransform.fromString(dateString!)
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
