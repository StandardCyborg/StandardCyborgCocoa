//
//  ServerAPIClient.swift
//  StandardCyborgNetworking
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import PromiseKit

internal struct SuccessResponse: Codable {
    let success: Bool
}

public enum Result<T> {
    case success(T)
    case failure(Error)
}

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
    
    /** Makes a request on the server, translating the specified dictionary into a JSON stringified HTTP body. Upon completion
        the response will be deserialized into an instance of an object of type T. */
    func performJSONOperation<T: Codable>(withURL url: URL,
                                          httpMethod: HTTPMethod,
                                          httpBodyDict: [AnyHashable: Any]?,
                                          responseObjectRootKey: String?,
                                          completion: @escaping (Result<T>) -> Void)
    
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
    func setResponseHeaders(on request: inout URLRequest) {
        headerValues.forEach { request.setValue($0.value, forHTTPHeaderField: $0.key) }
    }
}
