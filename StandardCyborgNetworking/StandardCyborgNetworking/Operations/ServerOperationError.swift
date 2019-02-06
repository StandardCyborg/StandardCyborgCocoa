//
//  ServerOperationError.swift
//  StandardCyborgNetworking
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

import Foundation

public enum ServerOperationError: Error {
    
    case networkOffline
    case sessionExpired
    case sessionInvalid
    case genericError(Error)
    case genericErrorString(String)
    case httpError(code: Int)
    case invalidUsernamePassword
    case invalidInput(message: String)
    
    var localizedDescription: String {
        switch self {
        case .networkOffline:
            return "Network Offline"
        case .sessionExpired:
            return "Please sign out and sign back in again\n(Session Expired)"
        case .sessionInvalid:
            return "Please sign out and sign back in again\n(Session Invalid)"
        case .genericError(let error):
            return error.localizedDescription
        case .genericErrorString(let errorString):
            return errorString
        case .httpError(let code):
            return "HTTP Error \(code)"
        case .invalidUsernamePassword:
            return "Invalid username or password"
        case .invalidInput(let message):
            return message
        }
    }
}

extension ServerOperationError: Equatable {
    
    public static func == (left: ServerOperationError, right: ServerOperationError) -> Bool {
        switch (left, right) {
        case (.networkOffline, .networkOffline):
            return true
        case (.sessionExpired, .sessionExpired):
            return true
        case (.sessionInvalid, .sessionInvalid):
            return true
        case (.genericError(let leftError), .genericError(let rightError)):
            return leftError.localizedDescription == rightError.localizedDescription
        case (.genericErrorString(let leftString), .genericErrorString(let rightString)):
            return leftString == rightString
        case (.httpError(let leftCode), .httpError(let rightCode)):
            return leftCode == rightCode
        case (.invalidInput(let leftMessage), .invalidInput(let rightMessage)):
            return leftMessage == rightMessage
        default:
            return false
        }
    }
    
}

