//
//  ServerOperation.swift
//  StandardCyborgNetworking
//
//  Created by Aaron Thompson on 8/15/19.
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

import Foundation

public class ServerOperation {
    
    let dataSource: ServerSyncEngineLocalDataSource
    let serverAPIClient: ServerAPIClient

    public init(dataSource: ServerSyncEngineLocalDataSource, serverAPIClient: ServerAPIClient) {
        self.dataSource = dataSource
        self.serverAPIClient = serverAPIClient
    }
    
}
