//
//  Credentials.swift
//  StandardCyborgNetworking
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation

fileprivate let MinimumSecondsToExpiration: TimeInterval = 2 * 60 * 60 // 2 hours


// These credentials are specific to interfacing with the Standard Cyborg server
// They're effectively two-legged OAuth2

// Protocol for persistence is broken out from ServerCredentials so we aren't limited to using ServerCredentials
// only as a generic constraint.
protocol ServerCredentialPersistence: Codable {
    static var defaultsKey: String { get }

    static func fromDefaults() -> Self?
    static func saveToDefaults(credentials: Self)
    static func invalidate()
}

extension ServerCredentialPersistence {
    static func fromDefaults() -> Self? {
        guard let data = UserDefaults.standard.data(forKey: Self.defaultsKey) else { return nil }
        return try! JSONDecoder().decode(Self.self, from: data)
    }

    static func saveToDefaults(credentials: Self) {
        let data = try! JSONEncoder().encode(credentials)
        UserDefaults.standard.set(data, forKey: Self.defaultsKey)
    }

    static func invalidate() {
        UserDefaults.standard.set(nil, forKey: Self.defaultsKey)
    }
}

protocol ServerCredentials: Codable {
    var accessToken: String { get }
    var isExpired: Bool { get }
    var isValid: Bool { get }

    var headerValues: [String : String] { get }
    mutating func updateAndPersistFromResponseHeaders(_ responseHeaders: [AnyHashable: Any])
}


extension ServerCredentials {
    var isValid: Bool {
        return !isExpired
    }
}

struct PersonalCredentials: ServerCredentials, ServerCredentialPersistence {
    static var defaultsKey: String { return "PersonalCredentials" }

    var expiry: String
    var tokenType: String
    var accessToken: String
    var client: String
    var uid: String

    var isExpired: Bool {
        let nowEpochSeconds = Date().timeIntervalSince1970
        let secondsRemaining = (expiry as NSString).doubleValue - nowEpochSeconds

        return secondsRemaining < MinimumSecondsToExpiration
    }

    var headerValues: [String : String] {
        return [
            ResponseHeader.accessToken.rawValue : accessToken,
            ResponseHeader.client.rawValue      : client,
            ResponseHeader.expiry.rawValue      : expiry,
            ResponseHeader.tokenType.rawValue   : tokenType,
            ResponseHeader.uid.rawValue         : uid
        ]
    }

    static func fromDefaults() -> PersonalCredentials? {
        // Check for our old credentials and migrate them to the new method before trying to read
        // that from defaults.
        if let credentialsToMigrate = _credentialsToMigrate() {
            _clearOldCredentials()
            return credentialsToMigrate
        }

        guard let data = UserDefaults.standard.data(forKey: defaultsKey) else { return nil }
        return try! JSONDecoder().decode(PersonalCredentials.self, from: data)
    }

    mutating func updateAndPersistFromResponseHeaders(_ responseHeaders: [AnyHashable : Any]) {
        guard
            let expiry = responseHeaders[ResponseHeader.expiry.rawValue] as? String,
            let tokenType = responseHeaders[ResponseHeader.tokenType.rawValue] as? String,
            let accessToken = responseHeaders[ResponseHeader.accessToken.rawValue] as? String,
            let client = responseHeaders[ResponseHeader.client.rawValue] as? String,
            let uid = responseHeaders[ResponseHeader.uid.rawValue] as? String else {
                print("Could not update personal credentials for response header: \(responseHeaders)")
                return
        }

        self.expiry = expiry
        self.tokenType = tokenType
        self.accessToken = accessToken
        self.client = client
        self.uid = uid
        PersonalCredentials.saveToDefaults(credentials: self)
    }

    // Returns nil if there aren't any credentials to migrate otherwise create an instance of personal
    // credentials which can be migrated to the new version.
    private static func _credentialsToMigrate() -> PersonalCredentials? {
        let defaults = UserDefaults.standard
        guard
            let accessToken = defaults.string(forKey: ResponseHeader.accessToken.defaultsKey),
            let client = defaults.string(forKey: ResponseHeader.client.defaultsKey),
            let expiry = defaults.string(forKey: ResponseHeader.expiry.defaultsKey),
            let tokenType = defaults.string(forKey: ResponseHeader.tokenType.defaultsKey),
            let uid = defaults.string(forKey: ResponseHeader.uid.defaultsKey)
        else {
            return nil
        }

        return PersonalCredentials(expiry: expiry, tokenType: tokenType, accessToken: accessToken, client: client, uid: uid)
    }

    // Clears out the old UserDefault keys so we don't try to migrate them again.
    private static func _clearOldCredentials() {
        let defaults = UserDefaults.standard
        defaults.set(nil, forKey: ResponseHeader.accessToken.defaultsKey)
        defaults.set(nil, forKey: ResponseHeader.client.defaultsKey)
        defaults.set(nil, forKey: ResponseHeader.expiry.defaultsKey)
        defaults.set(nil, forKey: ResponseHeader.tokenType.defaultsKey)
        defaults.set(nil, forKey: ResponseHeader.uid.defaultsKey)

    }
}

struct TeamCredentials: ServerCredentials, ServerCredentialPersistence {
    enum Key: String, Codable {
        case accessToken = "Access-Token"
        case apiKey = "Api-Key"
    }
    static var defaultsKey: String { return "TeamCredentials" }
    var apiKey: String
    var accessToken: String

    var isExpired: Bool { return false }

    var headerValues: [String : String] {
        return [
            Key.accessToken.rawValue : accessToken,
            Key.apiKey.rawValue      : apiKey
        ]
    }

    mutating func updateAndPersistFromResponseHeaders(_ responseHeaders: [AnyHashable : Any]) {
        guard
            let apiKey = responseHeaders[Key.apiKey.rawValue] as? String,
            let accessToken = responseHeaders[Key.accessToken.rawValue] as? String
        else {
            print("Could not update team credentials for response header: \(responseHeaders)")
            return
        }

        self.accessToken = accessToken
        self.apiKey = apiKey
        TeamCredentials.saveToDefaults(credentials: self)
    }
}

private extension ResponseHeader {
    var defaultsKey: String {
        return "SC-" + rawValue
    }
}
