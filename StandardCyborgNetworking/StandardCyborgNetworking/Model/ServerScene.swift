//
//  ServerScene.swift
//  StandardCyborgNetworking
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

import Foundation

public struct ServerScene: Codable {
    
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
    
    public init(createdAt: Date?,
                key: String?,
                teamUID: String?,
                versions: [ServerSceneVersion])
    {
        self.createdAt = createdAt
        self.key = key
        self.teamUID = teamUID
        self.versions = versions
    }
    
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
    public var sceneURL: URL?
    public var thumbnailURL: URL?
    
    public init(createdAt: Date?,
                versionNumber: Int,
                parentVersionNumber: Int,
                key: String?,
                sceneUID: String?,
                sceneURL: URL?,
                thumbnailURL: URL?)
    {
        self.createdAt = createdAt
        self.versionNumber = versionNumber
        self.parentVersionNumber = parentVersionNumber
        self.key = key
        self.sceneUID = sceneUID
        self.sceneURL = sceneURL
        self.thumbnailURL = thumbnailURL
    }
    
    // MARK: - Codable
    
    public init(from decoder: Decoder) {
        let container = try! decoder.container(keyedBy: CodingKeys.self)
        
        createdAt = try! container.decodeDateStringIfPresent(forKey: CodingKeys.createdAt)
        versionNumber = try! container.decode(Int.self, forKey: CodingKeys.versionNumber)
        parentVersionNumber = try! container.decode(Int.self, forKey: CodingKeys.parentVersionNumber)
        key = try! container.decodeIfPresent(String.self, forKey: .key)
        sceneUID = try! container.decode(String.self, forKey: CodingKeys.sceneUid)
        sceneURL = try! container.decodeIfPresent(URL.self, forKey: CodingKeys.scenegraphUrl)
        thumbnailURL = try! container.decodeIfPresent(URL.self, forKey: CodingKeys.thumbnailUrl)
    }
    
    public func encode(to encoder: Encoder) {
        var container = encoder.container(keyedBy: CodingKeys.self)
        
        try? container.encodeDateAsStringIfPresent(createdAt, forKey: CodingKeys.createdAt)
        try? container.encode(versionNumber, forKey: .versionNumber)
        try? container.encode(parentVersionNumber, forKey: .parentVersionNumber)
        try? container.encodeIfPresent(key, forKey: .key)
        try? container.encodeIfPresent(sceneUID, forKey: .sceneUid)
        try? container.encodeIfPresent(sceneURL, forKey: .scenegraphUrl)
        try? container.encodeIfPresent(thumbnailURL, forKey: .thumbnailUrl)
    }
}

public struct ServerSceneAsset: Codable {
    
    private enum CodingKeys: String, CodingKey {
        case createdAt
        case asset_key = "uid"
        case scene_uid
    }
    
    public var localUUID = UUID()
    public var uploadStatus: UploadStatus?
    
    public var createdAt: Date?
    public let asset_key: String?
    public var scene_uid: String?
    
    public init(createdAt: Date?,
                asset_key: String?,
                scene_uid: String?)
    {
        self.createdAt = createdAt
        self.asset_key = asset_key
        self.scene_uid = scene_uid
    }
    
    // MARK: - Codable
    
    public init(from decoder: Decoder) {
        let container = try! decoder.container(keyedBy: CodingKeys.self)
        
        createdAt = try! container.decodeDateStringIfPresent(forKey: CodingKeys.createdAt)
        asset_key = try! container.decodeIfPresent(String.self, forKey: .asset_key)
        scene_uid = try! container.decodeIfPresent(String.self, forKey: CodingKeys.scene_uid)
    }
    
    public func encode(to encoder: Encoder) {
        var container = encoder.container(keyedBy: CodingKeys.self)
        
        try? container.encodeDateAsStringIfPresent(createdAt, forKey: CodingKeys.createdAt)
        try? container.encodeIfPresent(asset_key, forKey: .asset_key)
        try? container.encodeIfPresent(scene_uid, forKey: .scene_uid)
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
