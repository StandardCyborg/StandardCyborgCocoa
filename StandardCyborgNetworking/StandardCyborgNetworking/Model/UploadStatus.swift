//
//  UploadStatus.swift
//  StandardCyborgNetworking
//
//  Created by Aaron Thompson on 8/12/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

import Foundation

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

