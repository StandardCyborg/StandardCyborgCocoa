//
//  AppDelegate.swift
//  TrueDepthFusion
//
//  Created by Aaron Thompson on 8/12/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import ARKit
import UIKit

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {
	var window: UIWindow?
    
    func application(_ application: UIApplication, willFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]? = nil) -> Bool {
        reloadScans()
        
        return true
    }
    
    private(set) var scans: [Scan] = []
    
    private var _scansContainerURL: URL {
        return URL(fileURLWithPath: NSHomeDirectory().appending("/Documents"))
    }
    
    func reloadScans() {
        let urls = try! FileManager.default.contentsOfDirectory(at: _scansContainerURL, includingPropertiesForKeys: nil, options: [])
        let plyURLs = urls
            .filter { $0.pathExtension == "ply" }
            .filter { !$0.lastPathComponent.contains("-mesh") }
        
        scans = plyURLs.map { url in Scan(plyPath: url.path) }
                .sorted { $0.dateCreated.compare($1.dateCreated) == .orderedDescending }
    }
    
    func add(_ scan: Scan) {
        if scan.plyPath == nil {
            do {
                try scan.write(toContainerPath: _scansContainerURL.path)
                scans.insert(scan, at: 0)
            } catch {
                print("Error saving scan: \(error)")
            }
        }
    }
    
    func remove(_ scan: Scan) {
        if let index = scans.firstIndex(of: scan) {
            do {
                try scan.deleteFiles()
                scans.remove(at: index)
            } catch {
                print("Error deleting files: \(error)")
            }
        }
    }
    
    func createBPLYScanDirectory() -> String {
        let directoryName = Scan.string(from: Date())
        let absoluteDirectory = _scansContainerURL.appendingPathComponent(directoryName)
        
        try? FileManager.default.createDirectory(at: absoluteDirectory, withIntermediateDirectories: false, attributes: nil)
        
        return absoluteDirectory.path
    }
    
}
