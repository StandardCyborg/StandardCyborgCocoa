//
//  ScenePreviewViewController.swift
//  Capture
//
//  Copyright © 2018 Apple. All rights reserved.
//

import Foundation
import SceneKit
import StandardCyborgFusion
import UIKit

@objc public class ScenePreviewViewController: UIViewController, SCNSceneRendererDelegate {
    @objc public private(set) var scScene: SCScene
    @objc private var meshTexturing: SCMeshTexturing?
    @objc private var landmarks: Set<SCLandmark3D>?
    
    /** A snapshot of the scene as-rendered, which becomes available
        as soon as the view controller's view appears. If meshing is enabled this
        property will be updated once the mesh has rendered.
     */
    @objc public private(set) var renderedSceneImage: UIImage? {
        didSet {
            guard let renderedSceneImage = renderedSceneImage else { return }
            onRenderedSceneImageUpdated?(renderedSceneImage)
        }
    }

    /** Gives us a hook to capture the renderedPointCloudImage separate from when it may be set. */
    @objc public var onRenderedSceneImageUpdated: ((UIImage) -> Void)?
    
    /** Gives us a hook to know the textured mesh is generated. */
    @objc public var onTexturedMeshGenerated: ((SCMesh) -> Void)?
    
    override public var preferredStatusBarStyle: UIStatusBarStyle { .default }
    
    /**
     Create an instance of ScenePreviewViewController with a pointCloud, optional meshTexturing instance (if
     you wish to render a mesh), and landmarks.
     
     You can get the correct instance of `meshTexturing` from `ScanningViewController.meshTexturing`
     after scanning has completed in the delegate callback `ScanningViewController(_ controller:didScan pointCloud)`
     */
    @objc public init(pointCloud: SCPointCloud, meshTexturing: SCMeshTexturing?, landmarks: Set<SCLandmark3D>?) {
        self.scScene = SCScene(pointCloud: pointCloud, mesh: nil)
        self.meshTexturing = meshTexturing
        self.landmarks = landmarks
        
        super.init(nibName: nil, bundle: nil)
    }
    
    @objc public init(scScene: SCScene) {
        self.scScene = scScene
        super.init(nibName: nil, bundle: nil)
    }

    required init?(coder: NSCoder) {
        self.scScene = SCScene(pointCloud: nil, mesh: nil)
        super.init(coder: coder)
    }
        
    
    /** Owners may mutate this view to change its appearance and add nodes to its scene */
    @objc public let sceneView: SCNView = {
        guard let sceneURL = Bundle.scuiResourcesBundle.url(forResource: "ScenePreviewViewController", withExtension: "scn") else {
            fatalError("Could not find scene file for ScenePreviewViewController")
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
        
    /** Appears while a mesh is being processed. Owners may mutate this to customize its appearance */
    @objc public let meshingProgressView: UIProgressView = {
        let progressView = UIProgressView()
        progressView.progress = 0
        return progressView
    }()

    // MARK: - UIViewController
    
    override public func viewDidLoad() {
        super.viewDidLoad()
        
        _containerNode.name = "Container"
        sceneView.scene?.rootNode.addChildNode(_containerNode)
        sceneView.delegate = self
        _initialPointOfView = sceneView.pointOfView!.transform
        
        meshingProgressView.isHidden = true

        view.backgroundColor = UIColor.white
        view.addSubview(sceneView)
        view.addSubview(leftButton)
        view.addSubview(rightButton)
        view.addSubview(meshingProgressView)
    }
    
    override public func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        sceneView.pointOfView!.transform = _initialPointOfView
        
        leftButton.isHidden = leftButton.title(for: UIControl.State.normal)?.isEmpty ?? true
        rightButton.isHidden = rightButton.title(for: UIControl.State.normal)?.isEmpty ?? true
        
        _constructScene(withSCScene: scScene)
    }
        
    
    // MARK: - Layout
    
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
        
        let progressViewCenter = CGPoint(x: view.center.x, y: view.safeAreaInsets.top + (meshingProgressView.frame.height / 2) + 12)
        meshingProgressView.bounds = CGRect(x: 0, y: 0, width: view.bounds.width - 2 * buttonSpacing, height: 8)
        meshingProgressView.center = progressViewCenter
    }
    
    // MARK: - SCNSceneRendererDelegate
    
    public func renderer(_ renderer: SCNSceneRenderer, updateAtTime time: TimeInterval) {
        // Experimentally determined
        // Default FOV: 55º
        // Min FOV: 10º
        // Max FOV: 120º
        
        let currentFOV = sceneView.pointOfView!.camera!.fieldOfView
        let pointSize = 10.0 - 0.078 * currentFOV
        
        if let pointsElement = _containerNode.childNode(withName: "SCPointCloud", recursively: true)?.geometry?.elements.first {
            pointsElement.pointSize = pointSize
            pointsElement.minimumPointScreenSpaceRadius = pointSize
            pointsElement.maximumPointScreenSpaceRadius = pointSize
        }
    }
        
    // MARK: - Private
    
    private var _initialPointOfView = SCNMatrix4Identity
    private var _containerNode = SCNNode()
    private var _meshingHelper: SCMeshingHelper?
    
    private func _constructScene(withSCScene scScene: SCScene) {
        _containerNode.childNodes.forEach { $0.removeFromParentNode() }
        
        _containerNode.addChildNode(scScene.rootNode)
        
        if let pointCloud = scScene.pointCloud, let meshTexturing = meshTexturing, _containerNode.childNode(withName: "SCMesh", recursively: true) == nil {
            _processMeshTexturingIntoMesh(withPointCloud: pointCloud, meshTexturing: meshTexturing) { result in
                switch result {
                case .success(let mesh):
                    self.scScene = SCScene(pointCloud: pointCloud, mesh: mesh)
                    self._constructScene(withSCScene: self.scScene)
                    self.onTexturedMeshGenerated?(mesh)
                    
                case .failure(let error):
                    print("Error processing mesh: \(String(describing: error.localizedDescription))")
                }
            }
        }
        
        // This is a bit of a hack. There doesn't appear to be a good way to determine when a sceneView has finished
        // rendering nodes. Delaying half a second before grabbing a snapshot seems to work.
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
            self.renderedSceneImage = self.sceneView.snapshot()
        }
    }
        
    private func _processMeshTexturingIntoMesh(withPointCloud pointCloud: SCPointCloud,
                                               meshTexturing: SCMeshTexturing,
                                               completion: @escaping ((Result<SCMesh, Error>) -> Void)) {
        meshingProgressView.isHidden = false
        meshingProgressView.setProgress(0, animated: false)
        
        _meshingHelper = SCMeshingHelper(pointCloud: pointCloud, meshTexturing: meshTexturing)
        _meshingHelper?.processMesh { meshingStatus in
            DispatchQueue.main.async {
                switch meshingStatus {
                case .inProgress(let progress):
                    self.meshingProgressView.setProgress(progress, animated: false)
                case .failure(let error):
                    self.meshingProgressView.isHidden = true
                    completion(.failure(error))    // Error is guaranteed to be set for the failure case
                    
                case .success(let mesh):
                    self.meshingProgressView.isHidden = true
                    completion(.success(mesh))
                }
            }
        }
    }
}
