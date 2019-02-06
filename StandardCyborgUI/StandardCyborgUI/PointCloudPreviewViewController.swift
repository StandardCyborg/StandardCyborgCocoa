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
    @objc public convenience init(pointCloud: SCPointCloud) {
        self.init()
        self.pointCloud = pointCloud
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
    
    // MARK: - Public
    
    /** Set this or call the convenience initializer before presenting this view controller */
    @objc public var pointCloud: SCPointCloud? {
        didSet { _buildNodeFromPointCloud() }
    }
    
    /** A snapshot of the point cloud as rendered, which becomes available
        as soon as the view controller's view appears */
    @objc public private(set) var renderedPointCloudImage: UIImage?
    
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
    }
    
    override public func viewWillAppear(_ animated: Bool) {
        sceneView.pointOfView!.transform = _initialPointOfView
        
        leftButton.isHidden = leftButton.title(for: UIControl.State.normal)?.isEmpty ?? true
        rightButton.isHidden = rightButton.title(for: UIControl.State.normal)?.isEmpty ?? true
    }
    
    override public func viewDidAppear(_ animated: Bool) {
        self.renderedPointCloudImage = self.sceneView.snapshot()
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
    }
    
    // MARK: - SCNSceneRendererDelegate
    
    private func renderer(_ renderer: SCNSceneRenderer, updateAtTime time: TimeInterval) {
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
    
    private func _buildNodeFromPointCloud() {
        for child in _containerNode.childNodes {
            child.removeFromParentNode()
        }
        
        _pointCloudNode = pointCloud?.buildNode()
        _pointCloudNode?.name = "Point cloud"
        
        if let node = _pointCloudNode {
            _containerNode.addChildNode(node)
        }
    }
    
}
