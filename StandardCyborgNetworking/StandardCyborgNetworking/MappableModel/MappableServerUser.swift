//
//  MappableUser.swift
//  Scanner
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import ObjectMapper

extension ServerUser: Mappable {
    
    public init?(map: Map) {
        guard
            let user = map.JSON["user"] as? [String: Any],
            let key = user["uid"] as? String, !key.isEmpty,
            let email = user["email"] as? String, !email.isEmpty
            else {
                print("Failed to map user from JSON \(map.JSON)")
                return nil
        }
        
        self.init()
    }
    
    public mutating func mapping(map: Map) {
        key       <- map["user.uid"]
        team      <- map["user.team"]
        email     <- map["user.email"]
        name      <- map["user.name"]
    }
    
    // MARK: -
    
    public init?(_ jsonObject: Any?) {
        guard let mapped = Mapper<ServerUser>().map(JSONObject: jsonObject) else {
            return nil
        }
        
        self = mapped
    }
    
    public mutating func update(fromJSONObject JSONObject: Any?) {
        _ = Mapper<ServerUser>().map(JSONObject: JSONObject, toObject: self)
    }
    
}
