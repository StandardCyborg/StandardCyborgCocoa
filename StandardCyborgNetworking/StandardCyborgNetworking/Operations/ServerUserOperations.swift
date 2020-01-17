//
//  ServerUserOperations.swift
//  StandardCyborgNetworking
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
    static let me = "me"
}


public class ServerSignUpOperation: ServerOperation {
    
    let email: String
    let password: String
    
    public init(dataSource: ServerSyncEngineLocalDataSource,
                apiClient: ServerAPIClient,
                email: String,
                password: String)
    {
        self.email = email
        self.password = password
        super.init(dataSource: dataSource, serverAPIClient: apiClient)
    }
    
    public func perform(_ completion: @escaping (Result<ServerUser>) -> Void) {
        let postDictionary = [
            "email": email,
            "password": password
        ]
        let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.authSignUp)

        serverAPIClient.performJSONOperation(withURL: url,
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

public class ServerSignInOperation: ServerOperation {
    
    let email: String
    let password: String
    
    public init(dataSource: ServerSyncEngineLocalDataSource,
                apiClient: ServerAPIClient,
                email: String,
                password: String)
    {
        self.email = email
        self.password = password
        super.init(dataSource: dataSource, serverAPIClient: apiClient)
    }
    
    public func perform(_ completion: @escaping (Result<ServerUser>) -> Void) {
        let postDictionary = [
            "email": email,
            "password": password
        ]
        let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.authSignIn)
        serverAPIClient.performJSONOperation(withURL: url,
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

public class ServerSignOutOperation: ServerOperation {
    
    public func perform(_ completion: @escaping (ServerOperationError?) -> Void) {
        let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.authSignOut)
        
        Promise<Void> { seal in
            serverAPIClient.performBasicOperation(withURL: url, httpMethod: .DELETE) { (error: ServerOperationError?) in
                seal.resolve(error)
            }
        }.ensure {
            // Always sign out, regardless of the result of the network operation
            self.dataSource.resetUser()
            self.serverAPIClient.invalidateCredentials()
        }.done {
            completion(nil)
        }.catch { error in
            let serverError = error as? ServerOperationError ?? ServerOperationError.genericError(error)
            completion(serverError)
        }
    }
    
}

public class ServerGetSignedInUserInfoOperation: ServerOperation {
    
    public func perform(_ completion: @escaping (Result<ServerUser>) -> Void) {
        let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.me)
        serverAPIClient.performJSONOperation(withURL: url,
                                             httpMethod: .GET,
                                             httpBodyDict: nil,
                                             responseObjectRootKey: "user")
        { (result: Result<ServerUser>) in
            if case let .success(user) = result {
                self.dataSource.updateUser(user)
            }
            
            completion(result)
        }
    }
}

public class ServerGenerateAccessTokenOperation: ServerOperation {

    let apiKey: String

    public init(dataSource: ServerSyncEngineLocalDataSource, apiClient: ServerAPIClient, apiKey: String) {
        self.apiKey = apiKey
        super.init(dataSource: dataSource, serverAPIClient: apiClient)
    }

    public func perform(_ completion: @escaping (Result<ServerAccessToken>) -> Void) {
        let postDictionary = [
            "api_key": apiKey,
        ]
        let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.authGenerateAccessToken)
        serverAPIClient.performJSONOperation(withURL: url,
                                             httpMethod: .POST,
                                             httpBodyDict: postDictionary,
                                             responseObjectRootKey: nil,
                                             completion: completion)
    }
}

public class ServerGenerateScopedAccessTokenOperation {
    
    let dataSource: ServerSyncEngineLocalDataSource
    let serverAPIClient: ServerAPIClient
    let apiKey: String
    let apiSecret: String
    let scope: String
    
    public init(dataSource: ServerSyncEngineLocalDataSource,
         serverAPIClient: ServerAPIClient,
         apiKey: String,
         apiSecret: String,
         scope: String)
    {
        self.dataSource = dataSource
        self.serverAPIClient = serverAPIClient
        self.apiKey = apiKey
        self.apiSecret = apiSecret
        self.scope = scope
    }
    
    public func perform(_ completion: @escaping (StandardCyborgNetworking.Result<ServerAccessToken>) -> Void) {
        let postDictionary = [
            "api_key": apiKey,
            "api_secret": apiSecret,
            "scope": scope,
        ]
        
        let url = serverAPIClient.buildAPIURL(for: ClientAPIPath.authGenerateAccessToken)
        serverAPIClient.performJSONOperation(withURL: url,
                                             httpMethod: .POST,
                                             httpBodyDict: postDictionary,
                                             responseObjectRootKey: nil,
                                             completion: completion)
    }
}

public class ServerTeamSignInOperation: ServerOperation {

    let email: String
    let password: String
    let apiKey: String  // read from info.plist

    public init(dataSource: ServerSyncEngineLocalDataSource, apiClient: ServerAPIClient, email: String, password: String) {
        self.email = email
        self.password = password
        
        guard let apiKey = Bundle.main.object(forInfoDictionaryKey: "SC_API_KEY") as? String else {
            fatalError("SC_API_KEY not set in info.plist.")
        }
        
        self.apiKey = apiKey
        
        super.init(dataSource: dataSource, serverAPIClient: apiClient)
    }
    
    public func perform(_ completion: @escaping (Result<(ServerUser, ServerAccessToken)>) -> Void) {
        let signInOperation = ServerSignInOperation(dataSource: dataSource, apiClient: serverAPIClient, email: email, password: password)
        let accessTokenOperation = ServerGenerateAccessTokenOperation(dataSource: dataSource, apiClient: serverAPIClient, apiKey: apiKey)

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
                        self.serverAPIClient.invalidateCredentials()
                        completion(.failure(error))
                    }
                }
            case .failure(let error):
                completion(.failure(error))
            }
        }
    }
}
