//
//  ServerUserOperations.swift
//  Scanner
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import class PromiseKit.Promise

private struct ClientAPIPath {
    static let authSignUp = "auth"
    static let authSignIn = "auth/sign_in"
    static let authSignOut = "auth/sign_out"
    static let authGenerateAccessToken = "auth/generate_access_token"
}




public struct ServerSignUpOperation {
    
    let dataSource: ServerSyncEngineLocalDataSource
    let apiClient: ServerAPIClient
    let email: String
    let password: String
    
    public init(dataSource: ServerSyncEngineLocalDataSource,
                apiClient: ServerAPIClient,
                email: String,
                password: String)
    {
        self.dataSource = dataSource
        self.apiClient = apiClient
        self.email = email
        self.password = password
    }
    
    public func perform(_ completion: @escaping (Result<ServerUser>) -> Void) {
        let postDictionary = [
            "email": email,
            "password": password
        ]
        let url = apiClient.buildAPIURL(for: ClientAPIPath.authSignUp)

        apiClient.performJSONOperation(withURL: url,
                                       httpMethod: .POST,
                                       httpBodyDict: postDictionary,
                                       responseObjectRootKey: "user")
        { (result: Result<ServerUser>) in
            var modifiedResult = result
            if case var .success(user) = result {
                user.key = self.email // Maybe correct? Works for now.
                self.dataSource.updateUser(user)
                modifiedResult = Result.success(user)
            }

            completion(modifiedResult)
        }
    }
    
}

public struct ServerSignInOperation {
    
    let dataSource: ServerSyncEngineLocalDataSource
    let apiClient: ServerAPIClient
    let email: String
    let password: String
    
    public init(dataSource: ServerSyncEngineLocalDataSource,
                apiClient: ServerAPIClient,
                email: String,
                password: String)
    {
        self.dataSource = dataSource
        self.apiClient = apiClient
        self.email = email
        self.password = password
    }
    
    public func perform(_ completion: @escaping (Result<ServerUser>) -> Void) {
        let postDictionary = [
            "email": email,
            "password": password
        ]
        let url = apiClient.buildAPIURL(for: ClientAPIPath.authSignIn)
        apiClient.performJSONOperation(withURL: url,
                                       httpMethod: .POST,
                                       httpBodyDict: postDictionary,
                                       responseObjectRootKey: "user")
        { (result: Result<ServerUser>) in
            if case let .success(user) = result {
                self.dataSource.updateUser(user)
            }
            completion(result)
        }
    }
    
}

public struct ServerSignOutOperation {
    
    let dataSource: ServerSyncEngineLocalDataSource
    let apiClient: ServerAPIClient
    
    public init(dataSource: ServerSyncEngineLocalDataSource, apiClient: ServerAPIClient) {
        self.dataSource = dataSource
        self.apiClient = apiClient
    }
    
    public func perform(_ completion: @escaping (ServerOperationError?) -> Void) {
        let url = apiClient.buildAPIURL(for: ClientAPIPath.authSignOut)
        
        Promise<Void> { seal in
            apiClient.performBasicOperation(withURL: url, httpMethod: .DELETE) { (error: ServerOperationError?) in
                seal.resolve(error)
            }
        }.ensure {
            // Always sign out, regardless of the result of the network operation
            self.dataSource.resetUser()
            self.apiClient.invalidateCredentials()
        }.done {
            completion(nil)
        }.catch { error in
            let serverError = error as? ServerOperationError ?? ServerOperationError.genericError(error)
            completion(serverError)
        }
    }
    
}

public struct ServerGenerateAccessTokenOperation {

    let dataSource: ServerSyncEngineLocalDataSource
    let apiClient: ServerAPIClient
    let apiKey: String

    public init(dataSource: ServerSyncEngineLocalDataSource, apiClient: ServerAPIClient, apiKey: String) {
        self.dataSource = dataSource
        self.apiClient = apiClient
        self.apiKey = apiKey
    }

    public func perform(_ completion: @escaping (Result<ServerAccessToken>) -> Void) {
        let postDictionary = [
            "api_key": apiKey,
        ]
        let url = apiClient.buildAPIURL(for: ClientAPIPath.authGenerateAccessToken)
        apiClient.performJSONOperation(withURL: url,
                                       httpMethod: .POST,
                                       httpBodyDict: postDictionary,
                                       responseObjectRootKey: nil,
                                       completion: completion)
    }
}


public struct ServerTeamSignInOperation {

    let dataSource: ServerSyncEngineLocalDataSource
    let apiClient: ServerAPIClient
    let email: String
    let password: String
    let apiKey: String  // read from info.plist

    public init(dataSource: ServerSyncEngineLocalDataSource, apiClient: ServerAPIClient, email: String, password: String) {
        self.dataSource = dataSource
        self.apiClient = apiClient
        self.email = email
        self.password = password

        guard let apiKey = Bundle.main.object(forInfoDictionaryKey: "SC_API_KEY") as? String else {
            fatalError("SC_API_KEY not set in info.plist.")
        }

        self.apiKey = apiKey
    }

    public func perform(_ completion: @escaping (Result<(ServerUser, ServerAccessToken)>) -> Void) {
        let signInOperation = ServerSignInOperation(dataSource: dataSource, apiClient: apiClient, email: email, password: password)
        let accessTokenOperation = ServerGenerateAccessTokenOperation(dataSource: dataSource, apiClient: apiClient, apiKey: apiKey)

        // NOTE: If we end up having more methods like this, the internals using PromiseKit (or Combine
        // if we get this to iOS 13+) should be exposed to make composing these async operations
        // more straightforward.
        // For a single method we can tolerate reading a small async pyramid.
        // Internally the operation will see an Access-Token in the body of the response and update our credentials
        // object with that value (meaning all future operations will be authenticated with that particular team).
        signInOperation.perform { signInResult in
            switch signInResult {
            case .success(let user):
                accessTokenOperation.perform { accessTokenResult in
                    switch accessTokenResult {
                    case .success(let token):
                        completion(.success((user, token)))
                    case .failure(let error):
                        self.dataSource.resetUser()
                        self.apiClient.invalidateCredentials()
                        completion(.failure(error))
                    }
                }
            case .failure(let error):
                completion(.failure(error))
            }
        }
    }
}




