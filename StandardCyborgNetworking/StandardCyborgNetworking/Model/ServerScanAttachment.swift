import Foundation

public struct ServerScanAttachment: Codable {
    // Upload props
    public let fileKey: String
    public let thumbnailKey: String?
    public let kind: String?
    public let publiclyDownloadable: Bool?
    public let publiclyVisible: Bool?
    public let metadata: [String : Any]?

    // Download props
    public let createdAt: Date?
    public let uploadedAt: Date?
    public let fileUrl: String?
    public let thumbnailUrl: String?
    public let teamUid: String?
    public let collectionUid: String?

    enum CodingKeys: String, CodingKey {
        case fileKey, thumbnailKey, kind, publiclyDownloadable, publiclyVisible, metadata
        case createdAt, uploadedAt, fileUrl, thumbnailUrl, teamUid, collectionUid
    }

    public init(from decoder: Decoder) throws {
        let container = try! decoder.container(keyedBy: CodingKeys.self)

        fileKey                 = try! container.decode(String.self, forKey: .fileKey)
        thumbnailKey            = try container.decodeIfPresent(String.self, forKey: .thumbnailKey)
        kind                    = try container.decodeIfPresent(String.self, forKey: .kind)
        publiclyDownloadable    = try container.decodeIfPresent(Bool.self, forKey: .publiclyDownloadable)
        publiclyVisible         = try container.decodeIfPresent(Bool.self, forKey: .publiclyVisible)
        metadata                = try container.decode([String : Any].self, forKey: .metadata)

        if let createdAtString = (try? container.decodeIfPresent(String.self, forKey: .createdAt)) ?? nil {
            createdAt = DateTimeTransform.fromString(createdAtString)
        } else {
            createdAt = nil
        }

        if let uploadedAtString = (try? container.decodeIfPresent(String.self, forKey: .uploadedAt)) ?? nil {
            uploadedAt = DateTimeTransform.fromString(uploadedAtString)
        } else {
            uploadedAt = nil
        }

        fileUrl                 = try container.decodeIfPresent(String.self, forKey: .fileUrl)
        thumbnailUrl            = try container.decodeIfPresent(String.self, forKey: .thumbnailUrl)
        teamUid                 = try container.decodeIfPresent(String.self, forKey: .teamUid)
        collectionUid           = try container.decodeIfPresent(String.self, forKey: .collectionUid)
    }

    public func encode(to encoder: Encoder) throws {
        var container = encoder.container(keyedBy: CodingKeys.self)

        try! container.encode(fileKey, forKey: .fileKey)
        try? container.encodeIfPresent(thumbnailKey, forKey: .thumbnailKey)
        try? container.encodeIfPresent(kind, forKey: .kind)
        try? container.encodeIfPresent(publiclyDownloadable, forKey: .publiclyDownloadable)
        try? container.encodeIfPresent(publiclyVisible, forKey: .publiclyVisible)
        try? container.encode(metadata, forKey: .metadata)

        try? container.encodeIfPresent(fileUrl, forKey: .fileUrl)
        try? container.encodeIfPresent(thumbnailUrl, forKey: .thumbnailUrl)
        try? container.encodeIfPresent(teamUid, forKey: .teamUid)
        try? container.encodeIfPresent(collectionUid, forKey: .collectionUid)
    }
}

