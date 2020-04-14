//
//  ViewController.swift
//  StandardCyborgExample
//
//  Copyright © 2019 Standard Cyborg. All rights reserved.
//

import StandardCyborgUI
import StandardCyborgFusion
import UIKit

class ViewController: UIViewController, ScanningViewControllerDelegate {
    
    // MARK: - IBOutlets and IBActions
    
    @IBOutlet private weak var showScanButton: UIButton!
    
    @IBAction private func startScanning(_ sender: UIButton) {
        #if targetEnvironment(simulator)
        let alert = UIAlertController(title: "Simulator Unsupported", message: "There is no depth camera available on the iOS Simulator. Please build and run on an iOS device with TrueDepth", preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .default, handler: nil))
        present(alert, animated: true)
        #else
        let scanningVC = ScanningViewController()
        scanningVC.delegate = self
        scanningVC.modalPresentationStyle = .fullScreen
        present(scanningVC, animated: true)
        #endif
    }
    
    @IBAction private func showScan(_ sender: UIButton) {
        guard let pointCloud = lastScanPointCloud else { return }
        
        pointCloudPreviewVC.pointCloud = pointCloud
        pointCloudPreviewVC.leftButton.setTitle("Delete", for: UIControl.State.normal)
        pointCloudPreviewVC.rightButton.setTitle("Dismiss", for: UIControl.State.normal)
        pointCloudPreviewVC.leftButton.backgroundColor = UIColor(named: "DestructiveAction")
        pointCloudPreviewVC.rightButton.backgroundColor = UIColor(named: "DefaultAction")
        pointCloudPreviewVC.modalPresentationStyle = .fullScreen
        
        present(pointCloudPreviewVC, animated: true)
    }
    
    // MARK: - UIViewController
    
    override func loadView() {
        super.loadView()
        
        showScanButton.layer.borderColor = UIColor.white.cgColor
        showScanButton.imageView?.contentMode = .scaleAspectFill
    }
    
    override func viewDidLoad() {
        loadScan()
    }
    
    // MARK: - ScanningViewControllerDelegate
    
    func scanningViewControllerDidCancel(_ controller: ScanningViewController) {
        dismiss(animated: true)
    }
    
    func scanningViewController(_ controller: ScanningViewController, didScan pointCloud: SCPointCloud) {
        pointCloudPreviewVC.pointCloud = pointCloud
        pointCloudPreviewVC.leftButton.setTitle("Rescan", for: UIControl.State.normal)
        pointCloudPreviewVC.rightButton.setTitle("Save", for: UIControl.State.normal)
        pointCloudPreviewVC.leftButton.backgroundColor = UIColor(named: "DestructiveAction")
        pointCloudPreviewVC.rightButton.backgroundColor = UIColor(named: "SaveAction")
        
        controller.present(pointCloudPreviewVC, animated: false)
    }
    
    @objc private func previewLeftButtonTapped(_ sender: UIButton) {
        let isExistingScan = pointCloudPreviewVC.pointCloud == lastScanPointCloud
        
        if isExistingScan {
            // Delete
            deleteScan()
            dismiss(animated: true)
        } else {
            // Retake
            dismiss(animated: false)
        }
    }
    
    @objc private func previewRightButtonTapped(_ sender: UIButton) {
        let isExistingScan = pointCloudPreviewVC.pointCloud == lastScanPointCloud
        
        if isExistingScan {
            // Dismiss
            dismiss(animated: true)
        } else {
            // Save
            saveScan(pointCloud: pointCloudPreviewVC.pointCloud!, thumbnail: pointCloudPreviewVC.renderedPointCloudImage)
            dismiss(animated: true)
        }
    }
    
    // MARK: - Private
    
    private let dateFormatter: DateFormatter = {
        let formatter = DateFormatter()
        formatter.dateStyle = .medium
        formatter.timeStyle = .short
        return formatter
    }()
    
    private lazy var pointCloudPreviewVC: PointCloudPreviewViewController = {
        let previewVC: PointCloudPreviewViewController = PointCloudPreviewViewController()
        previewVC.leftButton.addTarget(self, action: #selector(previewLeftButtonTapped(_:)), for: UIControl.Event.touchUpInside)
        previewVC.rightButton.addTarget(self, action: #selector(previewRightButtonTapped(_:)), for: UIControl.Event.touchUpInside)
        return previewVC
    }()
    
    private var lastScanPointCloud: SCPointCloud?
    private var lastScanDate: Date?
    private var lastScanThumbnail: UIImage?
    
    private lazy var documentsURL = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first!
    private lazy var scanPLYURL = documentsURL.appendingPathComponent("Cat.ply")
    private lazy var scanThumbnailURL = documentsURL.appendingPathComponent("Cat.jpeg")
    
    // MARK: -
    
    private func updateUI() {
        if lastScanThumbnail == nil {
            showScanButton.layer.borderWidth = 0
            showScanButton.setTitle("no scan yet", for: UIControl.State.normal)
        } else {
            showScanButton.layer.borderWidth = 1
            showScanButton.setTitle(nil, for: UIControl.State.normal)
        }
        
        showScanButton.setImage(lastScanThumbnail, for: UIControl.State.normal)
    }
    
    private func loadScan() {
        let scanPLYPath = scanPLYURL.path
        let scanThumbnailPath = scanThumbnailURL.path
        let fileManager = FileManager.default
        
        if
            fileManager.fileExists(atPath: scanPLYPath),
            let plyAttributes = try? fileManager.attributesOfItem(atPath: scanPLYPath),
            let dateCreated = plyAttributes[FileAttributeKey.creationDate] as? Date,
            let pointCloud = SCPointCloud(plyPath: scanPLYPath),
            pointCloud.pointCount > 0
        {
            lastScanPointCloud = pointCloud
            lastScanDate = dateCreated
            lastScanThumbnail = UIImage(contentsOfFile: scanThumbnailPath)
        }
        
        updateUI()
    }
    
    private func saveScan(pointCloud: SCPointCloud, thumbnail: UIImage?) {
        pointCloud.writeToPLY(atPath: scanPLYURL.path)
        
        if  let thumbnail = thumbnail,
            let jpegData = thumbnail.jpegData(compressionQuality: 0.8)
        {
            try? jpegData.write(to: scanThumbnailURL)
        }
        
        lastScanPointCloud = pointCloud
        lastScanThumbnail = thumbnail
        lastScanDate = Date()
        
        updateUI()
    }
    
    private func deleteScan() {
        let fileManager = FileManager.default
        
        if fileManager.fileExists(atPath: scanPLYURL.path) {
            try? fileManager.removeItem(at: scanPLYURL)
        }
        
        if fileManager.fileExists(atPath: scanThumbnailURL.path) {
            try? fileManager.removeItem(at: scanThumbnailURL)
        }
        
        lastScanPointCloud = nil
        lastScanThumbnail = nil
        lastScanDate = nil
        
        updateUI()
    }
    
}
