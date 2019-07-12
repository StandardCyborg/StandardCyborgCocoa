//
//  ServerScan.swift
//  StandardCyborgNetworking
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

import Foundation

public struct ServerScan: Codable {
    
    public enum UploadStatus: RawRepresentable, Codable {
        public typealias RawValue = String

        case notUploaded
        case uploading(percentComplete: Double)
        case uploaded

        public var rawValue: String {
            switch self {
            case .notUploaded: return "notUploaded"
            case .uploading(let percentComplete): return "uploading_\(percentComplete)"
            case .uploaded: return "uploaded"
            }
        }

        public init?(rawValue: RawValue) {
            if rawValue == UploadStatus.notUploaded.rawValue {
                self = .notUploaded
            } else if rawValue == UploadStatus.uploaded.rawValue {
                self = .uploaded
            } else if rawValue.starts(with: "uploading_"), let percentComplete = Double(rawValue.split(separator: "_").last!) {
                self = .uploading(percentComplete: percentComplete)
            }

            return nil
        }
    }
    
    public var localUUID = UUID()
    public let key: String?

    public var createdAt: Date?
    public var uploadedAt: Date?
    public let tagList: [String]
    public let attachments: [ServerScanAttachment]

    public var uploadStatus: UploadStatus?

    private enum CodingKeys: String, CodingKey {
        case key = "uid"
        case createdAt, uploadedAt, tagList, attachments
    }

    public static func create(withLocalUUID localUUID: UUID, uploadStatus: UploadStatus) -> ServerScan {
        return ServerScan(localUUID: localUUID, uploadStatus: uploadStatus)
    }

    private init(localUUID: UUID, uploadStatus: UploadStatus) {
        self.localUUID = localUUID
        self.uploadStatus = uploadStatus

        self.key = nil
        self.createdAt = nil
        self.uploadedAt = nil
        self.tagList = []
        self.attachments = []
    }

    public init(from decoder: Decoder) {
        let container = try! decoder.container(keyedBy: CodingKeys.self)
        localUUID = UUID()
        key = try! container.decodeIfPresent(String.self, forKey: .key)

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

        self.tagList = try! container.decodeIfPresent([String].self, forKey: .tagList) ?? []
        self.attachments = try! container.decodeIfPresent([ServerScanAttachment].self, forKey: .attachments) ?? [ServerScanAttachment]()
    }

    public func encode(to encoder: Encoder) {
        var container = encoder.container(keyedBy: CodingKeys.self)

        try? container.encodeIfPresent(key, forKey: .key)

        if let createdAt = createdAt {
            try? container.encodeIfPresent(DateTimeTransform.toString(createdAt), forKey: .createdAt)
        }

        if let uploadedAt = uploadedAt {
            try? container.encodeIfPresent(DateTimeTransform.toString(uploadedAt), forKey: .uploadedAt)
        }

        try? container.encodeIfPresent(tagList, forKey: .tagList)
        try? container.encodeIfPresent(attachments, forKey: .attachments)
    }
}
