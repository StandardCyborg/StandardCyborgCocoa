//
//  MappableScan.swift
//  Scanner
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import ObjectMapper

extension ServerScan: Mappable {
    
    public init?(map: Map) {
        guard let key = map.JSON["uid"] as? String, !key.isEmpty else {
            return nil
        }
        
        self.init()
    }
    
    public mutating func mapping(map: Map) {
        key        <-  map["uid"]
        createdAt  <- (map["created_at"], DateTimeTransform())
        uploadedAt <- (map["uploaded_at"], DateTimeTransform())
    }
    
    mutating func update(fromJSONObject JSONObject: Any?) {
        self = Mapper<ServerScan>().map(JSONObject: JSONObject, toObject: self)
    }
    
    static func scans(fromJSONObject JSONObject: Any?) -> [ServerScan]? {
        return Mapper<ServerScan>().mapArray(JSONObject: JSONObject)
    }
    
}
