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
