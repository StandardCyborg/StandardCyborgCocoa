//
//  MappableCustomer.swift
//  StandardCyborgNetworking
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

import Foundation
import ObjectMapper

extension ServerCustomer: Mappable {
    
    public convenience init?(map: Map) {
        self.init()
    }
    
    public func mapping(map: Map) {
        key     <- map["key"]
        email   <- map["email"]
    }
    
}
