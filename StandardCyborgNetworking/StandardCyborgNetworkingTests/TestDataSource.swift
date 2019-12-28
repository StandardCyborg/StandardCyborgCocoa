//  TestDataSource.swift
//
//  Created by Aaron Thompson on 12/27/19.
//

import Foundation
import StandardCyborgNetworking

class TestDataSource: ServerSyncEngineLocalDataSource {
    func beginWriteTransaction() {}
    func endWriteTransaction() {}
    
    var user: ServerUser = ServerUser()
    func updateUser(_ user: ServerUser) {}
    func resetUser() {}
    
    func allServerScans() -> [ServerScan] { return [] }
    func add(_ scan: ServerScan) {}
    func update(_ scan: ServerScan) {}
    func didDownloadPLYFile(for scan: ServerScan, to url: URL) {}
    func delete(_ scan: ServerScan) {}
    
    func localPLYURL(for scan: ServerScan) -> URL? { return nil }
    func localThumbnailURL(for scan: ServerScan) -> URL? { return nil }
}
