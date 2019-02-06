//
//  DefaultServerAPIClient.swift
//  Scanner
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import PromiseKit
import PMKFoundation

public class DefaultServerAPIClient: NSObject, ServerAPIClient, URLSessionTaskDelegate {
    
    private static let _BaseURLString = "https://platform.standardcyborg.com"
    private static let _ClientAPIURLRootComponent = "/api/v1/"
    
    private let _baseURLString: String
    private var _credentials = ServerCredentials()
    private var _progressHandlersBySession: [URLSession: ProgressHandler] = [:]
    
    public override init() {
        _baseURLString = DefaultServerAPIClient._BaseURLString
    }
    
    // MARK: - ServerAPIClient
    
    public func invalidateCredentials() {
        _credentials.invalidate()
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
        }.map { _, urlResponse in
            try self._validateURLResponse(urlResponse)
        }.done { _ in
            completion(nil)
        }.ensure {
            UIApplication.shared.isNetworkActivityIndicatorVisible = false
        }.catch { error in
            let serverError = error as? ServerOperationError ?? ServerOperationError.genericError(error)
            completion(serverError)
        }
    }
    
    public func performJSONOperation(
        withURL url: URL,
        httpMethod: HTTPMethod,
        httpBodyDict: [AnyHashable: Any]?,
        completion: @escaping (ServerOperationError?, Any?) -> Void)
    {
        UIApplication.shared.isNetworkActivityIndicatorVisible = true
        
        firstly {
            return URLSession.shared.dataTask(.promise, with:
                try self._makeJSONURLRequest(url: url, httpMethod: httpMethod, httpBodyDict: httpBodyDict)
            )
        }.map { data, urlResponse in
            try self._validateURLResponse(urlResponse)
            try self._updateCredentialsFromURLResponse(urlResponse)
            
            return try JSONSerialization.jsonObject(with: data, options: [])
        }.done { jsonObject in
            completion(nil, jsonObject)
        }.ensure {
            UIApplication.shared.isNetworkActivityIndicatorVisible = false
        }.catch { error in
            let serverError = error as? ServerOperationError ?? ServerOperationError.genericError(error)
            completion(serverError, nil)
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
            try self._validateURLResponse(response, data: data)
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
    
    public var isValid: Bool { return _credentials.isValid }
    
    public func buildURLRequest(url: URL, httpMethod: HTTPMethod, extraHeaders: [String: Any]?) throws -> URLRequest {
        guard _credentials.isExpired == false else {
            throw ServerOperationError.sessionExpired
        }
        
        var request = URLRequest(url: url)
        request.httpMethod = httpMethod.rawValue
        _credentials.setResponseHeaders(on: &request)
        
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
    
    private func _validateURLResponse(_ urlResponse: URLResponse, data: Data? = nil) throws {
        let httpResponse = urlResponse as! HTTPURLResponse
        
        switch httpResponse.statusCode {
        case 1 ..< 300:
            break
        case 401:
            if _credentials.accessToken == nil {
                // DEV: Still probably not the best place to do this, but it's less terribvle than other band-aid fixes
                throw ServerOperationError.invalidUsernamePassword
            } else {
                throw ServerOperationError.sessionInvalid
            }
        case 403:
            throw ServerOperationError.sessionInvalid
        case 404:
            throw ServerOperationError.invalidUsernamePassword
        default:
            throw ServerOperationError.httpError(code: httpResponse.statusCode)
        }
    }
    
    private func _updateCredentialsFromURLResponse(_ urlResponse: URLResponse) throws {
        guard let httpResponse = urlResponse as? HTTPURLResponse else {
            throw ServerOperationError.genericErrorString("URL response was not an HTTP response for \(urlResponse.url?.absoluteString ?? "no URL")")
        }
        
        let responseHeaders = httpResponse.allHeaderFields
        
        if responseHeaders[ResponseHeader.accessToken.rawValue] != nil {
            _credentials.updateFromResponseHeaders(responseHeaders)
//            throw ServerOperationError.genericErrorString("Sign in failed. No access-token header was returned in the response.")
        }
    }
    
    // MARK: - URLSessionTaskDelegate
    
    @objc private func urlSession(_ session: URLSession, task: URLSessionTask, didSendBodyData bytesSent: Int64, totalBytesSent: Int64, totalBytesExpectedToSend: Int64) {
        if let progressHandler = _progressHandlersBySession[session] {
            DispatchQueue.main.async {
                progressHandler(Double(totalBytesSent) / Double(totalBytesExpectedToSend))
            }
        }
    }
    
}
