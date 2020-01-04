//
//  DefaultServerAPIClient.swift
//  StandardCyborgNetworking
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import PromiseKit
#if canImport(PMKFoundation)
import PMKFoundation
#endif
import UIKit

public class DefaultServerAPIClient: NSObject, ServerAPIClient, URLSessionTaskDelegate {
    
    private static let _BaseURLString = "https://platform.standardcyborg.com"
    private static let _ClientAPIURLRootComponent = "/api/v2/"
    
    private let _baseURLString = DefaultServerAPIClient._BaseURLString
    private var _personalCredentials = PersonalCredentials.fromDefaults()
    private var _teamCredentials = TeamCredentials.fromDefaults()
    private var _progressHandlersBySession: [URLSession: ProgressHandler] = [:]
    
    private var _currentCredentials: ServerCredentials? {
        return _teamCredentials ?? _personalCredentials
    }

    // MARK: - ServerAPIClient
    
    public func invalidateCredentials() {
        PersonalCredentials.invalidate()
        TeamCredentials.invalidate()
        _personalCredentials = nil
        _teamCredentials = nil
    }
    
    public func buildAPIURL(for urlComponentString: String) -> URL {
        var urlString = _baseURLString
        urlString += DefaultServerAPIClient._ClientAPIURLRootComponent
        urlString += urlComponentString
        
        return URL(string: urlString)!
    }
    
    public func performBasicOperation(withURL url: URL,
                                      httpMethod: HTTPMethod,
                                      completion: @escaping (ServerOperationError?) -> Void)
    {
        UIApplication.shared.isNetworkActivityIndicatorVisible = true
        
        firstly {
            URLSession.shared.dataTask(.promise, with: try buildURLRequest(url: url, httpMethod: httpMethod, extraHeaders: nil))
        }.map { data, urlResponse in
            try self._validateURLResponse(urlResponse, data)
        }.done { _ in
            completion(nil)
        }.ensure {
            UIApplication.shared.isNetworkActivityIndicatorVisible = false
        }.catch { error in
            let serverError = error as? ServerOperationError ?? ServerOperationError.genericError(error)
            completion(serverError)
        }
    }

    public func performJSONOperation<T>(
        withURL url: URL,
        httpMethod: HTTPMethod,
        httpBodyDict: [AnyHashable: Any]?,
        responseObjectRootKey: String? = nil,
        completion: @escaping (Result<T>) -> Void) where T: Codable
    {
        UIApplication.shared.isNetworkActivityIndicatorVisible = true

        // Note for some of the promise closures we explicity define the type since the Swift
        // compiler will throw bizarre errors otherwise (was getting
        // "Use of '=' in a boolean context, did you mean '=='?" in the `ensure` block.
        firstly {
            return URLSession.shared.dataTask(.promise, with:
                try self._makeJSONURLRequest(url: url, httpMethod: httpMethod, httpBodyDict: httpBodyDict)
            )
        }.map { data, urlResponse in
            try self._validateURLResponse(urlResponse, data)
            try self._updatePersonalCredentialsFromURLResponse(urlResponse)

            // Technically we don't always need deserialize the json object, but this
            // approach is much easier to understand and since the json payloads are
            // small it's not a big deal to eat the performance hit here.
            return try JSONSerialization.jsonObject(with: data, options: [])
        }.compactMap { (jsonObject: Any) in
            guard let key = responseObjectRootKey else { return jsonObject }

            return (jsonObject as? [String : Any])?[key]
        }.map { (unwrappedJSONObject: Any) in
            // Look at the body of the response. If it has an access token key we should update our team credentials
            // to use that token.
            // NOTE: This approach WILL NOT work with an app wanting to support access to multiple teams at once.
            self._updateTeamCredentialsFromResponseBody(unwrappedJSONObject)

            let data = try JSONSerialization.data(withJSONObject: unwrappedJSONObject, options: [])
            let decoder = JSONDecoder()
            decoder.keyDecodingStrategy = .convertFromSnakeCase
            return try decoder.decode(T.self, from: data)
        }.done { (value: T) in
            completion(.success(value))
        }.ensure {
            UIApplication.shared.isNetworkActivityIndicatorVisible = false
        }.catch { error in
            let serverError = error as? ServerOperationError ?? ServerOperationError.genericError(error)
            completion(.failure(serverError))
        }
    }
    
    public func performDataUploadOperation(
        withURL url: URL,
        httpMethod: HTTPMethod,
        dataURL: URL,
        extraHeaders: [String: Any]?,
        progressHandler: ProgressHandler?,
        completion: @escaping (ServerOperationError?) -> Void)
    {
        UIApplication.shared.isNetworkActivityIndicatorVisible = true
        
        let session = URLSession(configuration: URLSessionConfiguration.default, delegate: self, delegateQueue: nil)
        _progressHandlersBySession[session] = progressHandler
        
        firstly {
            session.uploadTask(.promise,
                               with: try self.buildURLRequest(url: url, httpMethod: httpMethod, extraHeaders: extraHeaders),
                               fromFile: dataURL)
        }.map { (data, response) in
            try self._validateURLResponse(response, data)
        }.done { _ in
            completion(nil)
        }.ensure {
            UIApplication.shared.isNetworkActivityIndicatorVisible = false
            self._progressHandlersBySession[session] = nil
        }.catch { error in
            let serverError = error as? ServerOperationError ?? ServerOperationError.genericError(error)
            completion(serverError)
        }
    }
    
    public func performDataDownloadOperation(
        withURL url: URL,
        httpMethod: HTTPMethod,
        destinationURL: URL,
        extraHeaders: [String: Any]?,
        progressHandler: ProgressHandler?,
        completion: @escaping (ServerOperationError?) -> Void)
    {
        UIApplication.shared.isNetworkActivityIndicatorVisible = true
        
        let session = URLSession(configuration: URLSessionConfiguration.default, delegate: self, delegateQueue: nil)
        _progressHandlersBySession[session] = progressHandler
        
        firstly {
            session.downloadTask(.promise,
                                 with: try self.buildURLRequest(url: url, httpMethod: httpMethod, extraHeaders: extraHeaders),
                                 to: destinationURL)
        }.map { (_, response) in
            try self._validateURLResponse(response)
        }.done { _ in
            completion(nil)
        }.ensure {
            UIApplication.shared.isNetworkActivityIndicatorVisible = false
            self._progressHandlersBySession[session] = nil
        }.catch { error in
            let serverError = error as? ServerOperationError ?? ServerOperationError.genericError(error)
            completion(serverError)
        }
    }
    
    public var isValid: Bool { return _currentCredentials?.isValid ?? false }
    
    public func buildURLRequest(url: URL, httpMethod: HTTPMethod, extraHeaders: [String: Any]?) throws -> URLRequest {
        if let _currentCredentials = _currentCredentials, _currentCredentials.isExpired {
            throw ServerOperationError.sessionExpired
        }
        
        var request = URLRequest(url: url)
        request.httpMethod = httpMethod.rawValue

        // Set headers if we have them.
        _currentCredentials?.setResponseHeaders(on: &request)

        for (key, value) in extraHeaders ?? [:] {
            var valueString = value as? String
            
            if valueString == nil, let valueInt = value as? Int {
                valueString = String(format: "%d", valueInt)
            }
            
            request.setValue(valueString, forHTTPHeaderField: key)
        }
        
        return request
    }
    
    // MARK: - Private
    
    private func _makeJSONURLRequest(url: URL, httpMethod: HTTPMethod, httpBodyDict: [AnyHashable: Any]?) throws -> URLRequest {
        var request = try buildURLRequest(url: url, httpMethod: httpMethod, extraHeaders: nil)
        
        request.setValue("application/json", forHTTPHeaderField: "Content-Type")
        request.setValue("application/json", forHTTPHeaderField: "Accept")
        
        if let httpBodyDict = httpBodyDict {
            request.httpBody = try JSONSerialization.data(withJSONObject: httpBodyDict, options: [])
        }
        
        return request
    }
    
    private func _validateURLResponse(_ urlResponse: URLResponse, _ data: Data? = nil) throws {
        let httpResponse = urlResponse as! HTTPURLResponse
        
        switch httpResponse.statusCode {
        case 1 ..< 300:
            break
        case 401:
            if _currentCredentials?.accessToken == nil {
                // DEV: Still probably not the best place to do this, but it's less terrible than other band-aid fixes
                throw ServerOperationError.invalidUsernamePassword
            } else {
                throw ServerOperationError.sessionInvalid
            }
        case 403:
            throw ServerOperationError.sessionInvalid
        case 404:
            throw ServerOperationError.invalidUsernamePassword
        default:
            if let errorMessages = _errorMessagesFromData(data) {
                throw ServerOperationError.httpErrorMessages(code: httpResponse.statusCode, messages: errorMessages)
            } else {
                throw ServerOperationError.httpError(code: httpResponse.statusCode)
            }
        }
    }
    
    private func _errorMessagesFromData(_ data: Data?) -> [String]? {
        guard
            let data = data,
            let jsonObject = try? JSONSerialization.jsonObject(with: data, options: []),
            let jsonDict = jsonObject as? [String:Any],
            let errors = jsonDict["errors"] as? [String:Any],
            let fullMessages = errors["full_messages"] as? [String]
        else { return nil }
        
        return fullMessages
    }
    
    private func _updatePersonalCredentialsFromURLResponse(_ urlResponse: URLResponse) throws {
        guard let httpResponse = urlResponse as? HTTPURLResponse else {
            throw ServerOperationError.genericErrorString("URL response was not an HTTP response for \(urlResponse.url?.absoluteString ?? "no URL")")
        }
        
        let responseHeaders = httpResponse.allHeaderFields

        if responseHeaders[ResponseHeader.client.rawValue] != nil {
            // Create an instance here since it will be updated immediately after. An empty initialization let's
            // us avoid needing to duplicate the logic to pull the values out of the headers.
            // This could also be moved to an init method if desired.
            if self._personalCredentials == nil {
                _personalCredentials = PersonalCredentials(expiry: "", tokenType: "", accessToken: "", client: "", uid: "")
            }
            _personalCredentials?.updateAndPersistFromResponseHeaders(responseHeaders)
        }
    }

    private func _updateTeamCredentialsFromResponseBody(_ responseBody: Any) {
        guard
            let dict = responseBody as? [String : Any],
            let accessToken = dict[TeamCredentials.Key.accessToken.rawValue] as? String,
            let apiKey = dict[TeamCredentials.Key.apiKey.rawValue] as? String
        else {
            return
        }

        if self._teamCredentials == nil {
            _teamCredentials = TeamCredentials(apiKey: apiKey, accessToken: accessToken)
        }

        // This is technically redundant since we may have just created the credentials above, but
        // it handles the persistence for us as well as the case where we generate a token after
        // already having generated one.
        _teamCredentials?.updateAndPersistFromResponseHeaders(dict)
    }
    
    // MARK: - URLSessionTaskDelegate
    
    @objc public func urlSession(_ session: URLSession, task: URLSessionTask, didSendBodyData bytesSent: Int64, totalBytesSent: Int64, totalBytesExpectedToSend: Int64) {
        if let progressHandler = _progressHandlersBySession[session] {
            DispatchQueue.main.async {
                progressHandler(Double(totalBytesSent) / Double(totalBytesExpectedToSend))
            }
        }
    }
}
