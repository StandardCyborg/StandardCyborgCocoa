//
//  ServerScan.swift
//  StandardCyborgNetworking
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

import Foundation

public struct ServerScan {
    
    public enum UploadStatus {
        case notUploaded
        case uploading(percentComplete: Double)
        case uploaded
    }
    
    public var localUUID = UUID()
    public var key: String?
    public var createdAt: Date?
    public var uploadedAt: Date?
    public var uploadStatus: UploadStatus?
    
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
    
}
