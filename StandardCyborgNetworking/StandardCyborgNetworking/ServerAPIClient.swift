//
//  ServerAPIClient.swift
//  Scanner
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import ObjectMapper
import PromiseKit

public enum HTTPMethod: String {
    case GET
    case POST
    case PATCH
    case PUT
    case DELETE
}

/** HTTP response headers returned by the server with every request while signed in.
    These must be passed by the client with every request. */
public enum ResponseHeader: String {
    case accessToken = "Access-Token"
    case client = "Client"
    case expiry = "Expiry"
    case tokenType = "Token-Type"
    case uid = "Uid"
}

public typealias ProgressHandler = (Double) -> Void

/** Server operations use this API client to perform their work.
    Normally implemented by DefaultServerAPIClient, this could also be
    implemented by a different class for unit testing, for example.
 */
public protocol ServerAPIClient {
    
    /** Whether the current session is valid */
    var isValid: Bool { get }
    
    /** Call this upon signing out */
    func invalidateCredentials()
    
    /** Build a full URL for the specified path on the server */
    func buildAPIURL(for urlComponentString: String) -> URL
    
    /** Build a URLRequest with the specified parameters and HTTP method */
    func buildURLRequest(url: URL, httpMethod: HTTPMethod, extraHeaders: [String: Any]?) throws -> URLRequest
    
    /** Makes a request on the server with no HTTP body */
    func performBasicOperation(withURL url: URL,
                               httpMethod: HTTPMethod,
                               completion: @escaping (ServerOperationError?) -> Void)
    
    /** Makes a request on the server, translating the specified dictionary into a JSON stringified HTTP body */
    func performJSONOperation(withURL url: URL,
                              httpMethod: HTTPMethod,
                              httpBodyDict: [AnyHashable: Any]?,
                              completion: @escaping (ServerOperationError?, Any?) -> Void)
    
    /** Performs a long-running HTTP upload request on the server */
    func performDataUploadOperation(withURL url: URL,
                                    httpMethod: HTTPMethod,
                                    dataURL: URL,
                                    extraHeaders: [String: Any]?,
                                    progressHandler: ProgressHandler?,
                                    completion: @escaping (ServerOperationError?) -> Void)
    
    /** Performs a long-running HTTP download request on the server */
    func performDataDownloadOperation(withURL url: URL,
                                      httpMethod: HTTPMethod,
                                      destinationURL: URL,
                                      extraHeaders: [String: Any]?,
                                      progressHandler: ProgressHandler?,
                                      completion: @escaping (ServerOperationError?) -> Void)
    
}

extension ServerCredentials {
    
    mutating func updateFromResponseHeaders(_ responseHeaders: [AnyHashable: Any]) {
        accessToken  = responseHeaders[ResponseHeader.accessToken.rawValue] as? String
        client       = responseHeaders[ResponseHeader.client.rawValue] as? String
        expiry       = responseHeaders[ResponseHeader.expiry.rawValue] as? String
        tokenType    = responseHeaders[ResponseHeader.tokenType.rawValue] as? String
        uid          = responseHeaders[ResponseHeader.uid.rawValue] as? String
        
        persist()
    }
    
    func setResponseHeaders(on request: inout URLRequest) {
        request.setValue(accessToken, forHTTPHeaderField: ResponseHeader.accessToken.rawValue)
        request.setValue(client,      forHTTPHeaderField: ResponseHeader.client.rawValue)
        request.setValue(expiry,      forHTTPHeaderField: ResponseHeader.expiry.rawValue)
        request.setValue(tokenType,   forHTTPHeaderField: ResponseHeader.tokenType.rawValue)
        request.setValue(uid,         forHTTPHeaderField: ResponseHeader.uid.rawValue)
    }
    
}
