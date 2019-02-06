//
//  Credentials.swift
//  Scanner
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation

// These credentials are specific to interfacing with the Standard Cyborg server
// They're effectively two-legged OAuth2
struct ServerCredentials {
    
    private static let MinimumSecondsToExpiration: TimeInterval = 2 * 60 * 60 // 2 hours
    
    var accessToken: String?
    var client: String?
    var expiry: String?
    var tokenType: String?
    var uid: String?
    
    init() {
        load()
    }
    
    mutating func load() {
        let defaults = UserDefaults.standard
        
        // DEV: To make things easy on ourselves, we use the response headers as keys
        accessToken = defaults.string(forKey: ResponseHeader.accessToken.defaultsKey)
        client = defaults.string(forKey: ResponseHeader.client.defaultsKey)
        expiry = defaults.string(forKey: ResponseHeader.expiry.defaultsKey)
        tokenType = defaults.string(forKey: ResponseHeader.tokenType.defaultsKey)
        uid = defaults.string(forKey: ResponseHeader.uid.defaultsKey)
    }
    
    func persist() {
        let defaults = UserDefaults.standard
        
        defaults.set(accessToken, forKey: ResponseHeader.accessToken.defaultsKey)
        defaults.set(client, forKey: ResponseHeader.client.defaultsKey)
        defaults.set(expiry, forKey: ResponseHeader.expiry.defaultsKey)
        defaults.set(tokenType, forKey: ResponseHeader.tokenType.defaultsKey)
        defaults.set(uid, forKey: ResponseHeader.uid.defaultsKey)
    }
    
    mutating func invalidate() {
        accessToken = nil
        client = nil
        expiry = nil
        tokenType = nil
        uid = nil
        
        persist()
    }
    
    var isValid: Bool {
        guard expiry != nil, uid != nil && expiry != nil && client != nil && tokenType != nil && accessToken != nil else {
            return false
        }
        
        return !isExpired
    }
    
    var isExpired: Bool {
        guard let expiryString = expiry else {
            return false
        }
        
        let nowEpochSeconds = Date().timeIntervalSince1970
        let secondsRemaining = (expiryString as NSString).doubleValue - nowEpochSeconds
        
        return secondsRemaining < ServerCredentials.MinimumSecondsToExpiration
    }
    
}

private extension ResponseHeader {
    var defaultsKey: String {
        return "SC-" + rawValue
    }
}
