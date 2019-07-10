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
    public var key: String?
    public var createdAt: Date?
    public var uploadedAt: Date?
    public var uploadStatus: UploadStatus?

    private enum CodingKeys: String, CodingKey {
        case key = "uid"
        case createdAt
        case uploadedAt
    }
    
    public init(
        localUUID: UUID = UUID(),
        key: String? = nil,
        createdAt: Date? = nil,
        uploadedAt: Date? = nil,
        uploadStatus: UploadStatus? = nil)
    {
        self.localUUID = localUUID
        self.key = key
        self.createdAt = createdAt
        self.uploadedAt = uploadedAt
        self.uploadStatus = uploadStatus
    }

    public init(from decoder: Decoder) {
        let container = try! decoder.container(keyedBy: CodingKeys.self)
        localUUID = UUID()
        key = try! container.decode(String.self, forKey: .key)

        let createdAtString = try! container.decode(String.self, forKey: .createdAt)
        createdAt = DateTimeTransform.fromString(createdAtString)

        let uploadedAtString = try! container.decode(String.self, forKey: .uploadedAt)
        uploadedAt = DateTimeTransform.fromString(uploadedAtString)
    }

    public func encode(to encoder: Encoder) {
        var container = encoder.container(keyedBy: CodingKeys.self)
        try! container.encode(key, forKey: .key)
        if let createdAt = createdAt {
            try! container.encode(DateTimeTransform.toString(createdAt), forKey: .createdAt)
        }

        if let uploadedAt = uploadedAt {
            try! container.encode(DateTimeTransform.toString(uploadedAt), forKey: .uploadedAt)
        }
    }

    internal func merge(withScan scan: ServerScan) -> ServerScan {
        return ServerScan(localUUID: localUUID, key: scan.key, createdAt: scan.createdAt, uploadedAt: scan.uploadedAt, uploadStatus: uploadStatus)
    }
}
