//
//  PointCloudPreviewViewController.swift
//  Capture
//
//  Copyright © 2018 Apple. All rights reserved.
//

import Foundation
import SceneKit
import StandardCyborgFusion
import UIKit

@objc public class PointCloudPreviewViewController: UIViewController, SCNSceneRendererDelegate {
    
    /** A convenience initializer that simply calls init and sets the point cloud */
    @objc public convenience init(pointCloud: SCPointCloud, meshTexturing: SCMeshTexturing, landmarks: Set<SCLandmark3D>?) {
        self.init()
        self.pointCloud = pointCloud
        self.meshTexturing = meshTexturing
        self.landmarks = landmarks
        _buildNodeFromPointCloud()
    }
    
    /** Owners may mutate this view to change its appearance and add nodes to its scene */
    @objc public let sceneView: SCNView = {
        guard let sceneURL = Bundle(for: PointCloudPreviewViewController.self).url(forResource: "PointCloudPreviewViewController", withExtension: "scn") else {
            fatalError("Could not find scene file for PointCloudPreviewViewController")
        }
        
        let scene = try! SCNScene(url: sceneURL, options: nil)
        scene.background.contents = UIColor.clear
        
        let sceneView = SCNView()
        sceneView.scene = scene
        sceneView.allowsCameraControl = true
        sceneView.backgroundColor = UIColor.clear
        return sceneView
    }()
    
    /** Owners may mutate this button to customize its appearance and respond to taps */
    @objc public let leftButton: UIButton = {
        let button = UIButton(type: UIButton.ButtonType.custom)
        button.setTitleColor(UIColor.black, for: UIControl.State.normal)
        button.titleLabel?.font = UIFont.systemFont(ofSize: 20, weight: UIFont.Weight.semibold)
        button.backgroundColor = UIColor(red: 1.0, green: 0.27, blue: 0.27, alpha: 1.0)
        button.layer.cornerRadius = 10
        return button
    }()
    
    /** Owners may mutate this button to customize its appearance and respond to taps */
    @objc public let rightButton: UIButton = {
        let button = UIButton(type: UIButton.ButtonType.custom)
        button.setTitleColor(UIColor.black, for: UIControl.State.normal)
        button.titleLabel?.font = UIFont.systemFont(ofSize: 20, weight: UIFont.Weight.semibold)
        button.backgroundColor = UIColor(red: 0.14, green: 0.54, blue: 1.0, alpha: 1.0)
        button.layer.cornerRadius = 10
        return button
    }()
    
    @objc public let meshToggle: UISwitch = {
        let toggle = UISwitch()
        toggle.isOn = false
        toggle.addTarget(self, action: #selector(_toggleMeshVisibility), for: .valueChanged)
        return toggle
    }()
    
    @objc public let meshingProgressView: UIProgressView = {
        let progressView = UIProgressView()
        progressView.progress = 0
        return progressView
    }()
    
    // MARK: - Public
    
    /** Set this or call the convenience initializer before presenting this view controller */
    @objc public var pointCloud: SCPointCloud?
    
    /**
     Set this before presenting this view controller if you wnat to render a mesh.
     
     You can get the correct instance of this object from `ScanningViewController.meshTexturing`
     after scanning has completed in the delegate callback `ScanningViewController(_ controller:didScan pointCloud)`
     */
    @objc public var meshTexturing: SCMeshTexturing?
    
    /** Set this or call the convenience initializer before presenting this view controller */
    @objc public var landmarks: Set<SCLandmark3D>?
    
    /** A snapshot of the point cloud as rendered, which becomes available
        as soon as the view controller's view appears */
    @objc public private(set) var renderedPointCloudImage: UIImage?

    /** Gives us a hook to capture the renderedPointCloudImage separate from when it may be set. */
    @objc public var onRenderedPointCloudImageReady: ((UIImage) -> Void)?

    // MARK: - UIViewController
    
    override public func viewDidLoad() {
        _containerNode.name = "Container"
        sceneView.scene?.rootNode.addChildNode(_containerNode)
        sceneView.delegate = self
        _initialPointOfView = sceneView.pointOfView!.transform
        
        view.backgroundColor = UIColor.white
        view.addSubview(sceneView)
        view.addSubview(leftButton)
        view.addSubview(rightButton)
        view.addSubview(meshToggle)
        view.addSubview(meshingProgressView)
    }
    
    override public func viewWillAppear(_ animated: Bool) {
        sceneView.pointOfView!.transform = _initialPointOfView
        
        if _pointCloudNode == nil && pointCloud != nil {
            _buildNodeFromPointCloud()
        }
        
        meshToggle.isHidden = true
        meshingProgressView.isHidden = true
        if _meshNode == nil && pointCloud != nil, meshTexturing != nil {
            _processMesh()
        }
        
        leftButton.isHidden = leftButton.title(for: UIControl.State.normal)?.isEmpty ?? true
        rightButton.isHidden = rightButton.title(for: UIControl.State.normal)?.isEmpty ?? true
    }
    
    override public func viewDidAppear(_ animated: Bool) {
        self.renderedPointCloudImage = self.sceneView.snapshot()
        onRenderedPointCloudImageReady?(self.renderedPointCloudImage!)
    }
    
    override public func viewDidLayoutSubviews() {
        sceneView.frame = view.bounds
        
        let buttonHeight: CGFloat = 56
        let buttonSpacing: CGFloat = 20
        let buttonInsets = UIEdgeInsets(top: 0, left: 20, bottom: 5, right: 20)
        let buttonCount = (leftButton.isHidden ? 0 : 1) + (rightButton.isHidden ? 0 : 1)
        
        if buttonCount > 0 {
            var buttonFrame = CGRect.zero
            buttonFrame.size.width = CGFloat(1) / CGFloat(buttonCount) * (view.bounds.width - buttonInsets.left - buttonInsets.right - CGFloat(buttonCount - 1) * buttonSpacing)
            buttonFrame.size.height = buttonHeight
            buttonFrame.origin.x = buttonInsets.left
            buttonFrame.origin.y = view.bounds.height - view.safeAreaInsets.bottom - buttonInsets.bottom - buttonHeight
            leftButton.frame = buttonFrame
            rightButton.frame = leftButton.isHidden ? buttonFrame : buttonFrame.offsetBy(dx: buttonFrame.width + buttonSpacing, dy: 0)
        }
        
        meshToggle.sizeToFit()
        meshToggle.center = CGPoint(x: view.center.x, y: view.safeAreaInsets.top + meshToggle.frame.height)
        meshingProgressView.frame = CGRect(x: meshToggle.frame.origin.x, y: meshToggle.frame.maxY + 10, width: meshToggle.frame.width, height: 8)
    }
    
    // MARK: - SCNSceneRendererDelegate
    
    public func renderer(_ renderer: SCNSceneRenderer, updateAtTime time: TimeInterval) {
        // Experimentally determined
        // Default FOV: 55º
        // Min FOV: 10º
        // Max FOV: 120º
        
        let currentFOV = sceneView.pointOfView!.camera!.fieldOfView
        let pointSize = 10.0 - 0.078 * currentFOV
        
        if let pointsElement = _pointCloudNode?.geometry?.elements.first {
            pointsElement.pointSize = pointSize
            pointsElement.minimumPointScreenSpaceRadius = pointSize
            pointsElement.maximumPointScreenSpaceRadius = pointSize
        }
    }
    
    // MARK: - Private
    
    private var _initialPointOfView = SCNMatrix4Identity
    private var _containerNode = SCNNode()
    private var _pointCloudNode: SCNNode?
    private var _meshNode: SCNNode?
    private var _meshingHelper: MeshingHelper?
    
    private func _buildNodeFromPointCloud() {
        for child in _containerNode.childNodes {
            child.removeFromParentNode()
        }
        
        _pointCloudNode = pointCloud?.buildNode(with: landmarks)
        _pointCloudNode?.name = "Point cloud"
        
        if let node = _pointCloudNode {
            _containerNode.addChildNode(node)
        }
    }
    
    private func _processMesh() {
        guard let pointCloud = pointCloud, let meshTexturing = meshTexturing else { return }
        
        meshingProgressView.isHidden = false
        meshingProgressView.setProgress(0, animated: false)
        
        _meshingHelper = MeshingHelper(pointCloud: pointCloud, meshTexturing: meshTexturing)
        _meshingHelper?.processMesh { meshingStatus in
            DispatchQueue.main.async {
                switch meshingStatus {
                case .inProgress(let progress):
                    self.meshingProgressView.setProgress(progress, animated: false)
                case .failure(let error):
                    print("Error processing mesh: \(String(describing: error?.localizedDescription))")
                case .success(let mesh):
                    self._buildMeshNodeWithMesh(mesh: mesh)
                    self.meshingProgressView.isHidden = true
                    self.meshToggle.isHidden = false
                    self.meshToggle.isOn = true
                }
            }
        }
    }
    
    private func _buildMeshNodeWithMesh(mesh: SCMesh) {
        let node = mesh.buildMeshNode()
        node.name = "SCMesh"
        node.position = _pointCloudNode!.position
        node.scale = _pointCloudNode!.scale
        _containerNode.addChildNode(node)
    }
    
    @objc private func _toggleMeshVisibility() {
        _containerNode.childNode(withName: "SCMesh", recursively: true)?.isHidden = !meshToggle.isOn
    }
}


private class MeshingHelper {
    enum Status {
        case inProgress(Float)
        case success(SCMesh)
        case failure(Error?)
    }

    private static var defaultMeshingParameters: SCMeshingParameters {
        let meshingParameters = SCMeshingParameters()
        meshingParameters.resolution = 5
        meshingParameters.smoothness = 2
        meshingParameters.surfaceTrimmingAmount = 6
        meshingParameters.closed = true
        return meshingParameters
    }
    
    let pointCloud: SCPointCloud
    let meshTexturing: SCMeshTexturing
    let meshingParameters: SCMeshingParameters
    
    init(pointCloud: SCPointCloud, meshTexturing: SCMeshTexturing, meshingParameters: SCMeshingParameters? = nil) {
        self.pointCloud = pointCloud
        self.meshTexturing = meshTexturing
        self.meshingParameters = meshingParameters ?? Self.defaultMeshingParameters
    }
     
    func processMesh(onMeshStatusUpdate: @escaping ((Status) -> Void)) {
        meshTexturing.reconstructMesh(
            pointCloud: pointCloud,
            textureResolution: 2048,
            meshingParameters: meshingParameters,
            progress: { (progress, stop) in
                onMeshStatusUpdate(.inProgress(progress))
            },
            completion: { (error, mesh) in
                if let mesh = mesh {
                    onMeshStatusUpdate(.success(mesh))
                    return
                }
                
                onMeshStatusUpdate(.failure(error))
        })
    }
}
