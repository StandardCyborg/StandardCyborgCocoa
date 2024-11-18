//
//  BPLYScanningViewController.swift
//  TrueDepthFusion
//
//  Created by Aaron Thompson on 11/7/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import ARKit
import CoreMotion
import MediaPlayer
import SceneKit
import StandardCyborgFusion
import UIKit

class BPLYScanningViewController: UIViewController, CameraManagerDelegate {
    
    private enum ScanningTerminationReason {
        case canceled
        case finished
    }
    
    // MARK: - Outlets and Actions
    
    @IBOutlet private weak var metalContainerView: UIView!
    @IBOutlet private weak var scanDurationContainerView: UIView!
    @IBOutlet private weak var scanDurationLabel: UILabel!
    @IBOutlet private weak var elapsedDurationLabel: UILabel!
    @IBOutlet private weak var shutterButton: UIButton!
    @IBOutlet private weak var countdownLabel: UILabel!
    
    // MARK: -
    
    @IBAction private func scanDurationChanged(_ sender: UISlider) {
        _scanDurationSeconds = Int(sender.value)
    }
    
    @IBAction private func shutterTapped(_ sender: UIButton?) {
        if _scanning {
            AudioAndHapticEngine.shared.scanningFinished()
            _stopScanning(reason: .finished)
        } else if _countdownSeconds > 0 {
            AudioAndHapticEngine.shared.scanningCanceled()
            _cancelCountdown()
        } else {
            _startCountdown { self._startScanning() }
        }
    }
    
    @IBAction func done(_ sender: UIButton) {
        _stopScanning(reason: .finished)
        
        presentingViewController?.dismiss(animated: true, completion: nil)
    }
    
    // MARK: - Properties
    
    private let _appDelegate = UIApplication.shared.delegate! as! AppDelegate
    private let _metalLayer = CAMetalLayer()
    private let _metalDevice = MTLCreateSystemDefaultDevice()!
    private lazy var _commandQueue = _metalDevice.makeCommandQueue()!
    private lazy var _reconstructionManager = SCReconstructionManager(device: _metalDevice, commandQueue: _commandQueue, maxThreadCount: 2)
    private lazy var _scanningViewRenderer = ScanningViewRenderer(device: _metalDevice, commandQueue: _commandQueue)
    private var _scanningTimer: Timer?
    private let _cameraManager = CameraManager()
    private let _motionManager = CMMotionManager()
    
    private var _useFullResolutionDepthFrames: Bool {
        get { return UserDefaults.standard.bool(forKey: "full_resolution_depth_frames", defaultValue: false) }
        set { UserDefaults.standard.set(newValue, forKey: "full_resolution_depth_frames") }
    }
    
    // MARK: - UIViewController
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        _metalLayer.isOpaque = true
        _metalLayer.contentsScale = UIScreen.main.scale
        _metalLayer.device = _metalDevice
        _metalLayer.pixelFormat = MTLPixelFormat.bgra8Unorm
        _metalLayer.framebufferOnly = false
        _metalLayer.frame = metalContainerView.bounds
        metalContainerView.layer.addSublayer(_metalLayer)
        metalContainerView.addGestureRecognizer(UITapGestureRecognizer(target: self, action: #selector(focusOnTap)))
        
        _cameraManager.delegate = self
        _cameraManager.configureCaptureSession(maxColorResolution: 1920,
                                               maxDepthResolution: _useFullResolutionDepthFrames ? 640 : 320,
                                               maxFramerate: 30)

        
        NotificationCenter.default.addObserver(self, selector: #selector(thermalStateChanged),
                                               name: ProcessInfo.thermalStateDidChangeNotification, object: nil)
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        _updateUI()
        
        _cameraManager.startSession { result in
            switch result {
            case .success:
                break
            case .configurationFailed:
                print("Configuration failed for an unknown reason")
            case .notAuthorized:
                let message = NSLocalizedString("TrueDepth Fusion doesn't have permission to use the camera, please change privacy settings",
                                                comment: "Alert message when the user has denied access to the camera")
                let alertController = UIAlertController(title: "TrueDepth Fusion", message: message, preferredStyle: .alert)
                alertController.addAction(UIAlertAction(title: NSLocalizedString("OK", comment: "Alert OK button"),
                                                        style: .cancel,
                                                        handler: nil))
                alertController.addAction(UIAlertAction(title: NSLocalizedString("Settings", comment: "Alert button to open Settings"),
                                                        style: .`default`)
                { _ in
                    UIApplication.shared.open(URL(string: UIApplication.openSettingsURLString)!, options: [:], completionHandler: nil)
                })
                
                self.present(alertController, animated: true, completion: nil)
            }
        }
        
        _motionManager.startDeviceMotionUpdates(to: OperationQueue.main) { [weak self] (motion: CMDeviceMotion?, error: Error?) in
            guard let motion = motion else { return }
            
            if self?._scanning ?? false {
                self?._bplyDepthDataAccumulator?.accumulate(deviceMotion: motion)
            }
        }
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        
        _cameraManager.stopSession()
        _motionManager.stopDeviceMotionUpdates()
    }
    
    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
        
        CATransaction.begin()
        CATransaction.disableActions()
        _metalLayer.frame = metalContainerView.bounds
        _metalLayer.drawableSize = CGSize( width: _metalLayer.frame.width  * _metalLayer.contentsScale,
                                          height: _metalLayer.frame.height * _metalLayer.contentsScale)
        CATransaction.commit()
    }
    
    override func didReceiveMemoryWarning() {
        _stopScanning(reason: .finished)
    }
    
    // MARK: - Notifications
    
    @objc private func focusOnTap(_ gesture: UITapGestureRecognizer) {
        guard !_scanning else { return }
        
        let location = gesture.location(in: metalContainerView)
        
        _cameraManager.focusOnTap(at: location)
    }
    
    @objc private func thermalStateChanged(notification: NSNotification) {
        guard let processInfo = notification.object as? ProcessInfo,
            processInfo.thermalState == .critical
            else { return }
        
        DispatchQueue.main.async {
            if self._scanning {
                self._stopScanning(reason: .finished)
            }
            
            let alertController = UIAlertController(title: "iPhone is too hot!",
                                                    message: "Please allow iPhone to cool down and try again",
                                                    preferredStyle: .alert)
            alertController.addAction(UIAlertAction(title: "OK", style: .cancel, handler: nil))
            self.present(alertController, animated: true, completion: nil)
        }
    }
    
    // MARK: - CameraManagerDelegate
    
    func cameraDidOutput(colorBuffer: CVPixelBuffer,
                         colorTime: CMTime,
                         depthBuffer: CVPixelBuffer,
                         depthTime: CMTime,
                         depthCalibrationData: AVCameraCalibrationData)
    {
        let pointCloud = _reconstructionManager.reconstructSingleDepthBuffer(depthBuffer,
                                                                             colorBuffer: colorBuffer,
                                                                             with: depthCalibrationData,
                                                                             smoothingPoints: !self._useFullResolutionDepthFrames)
        
        _scanningViewRenderer.draw(colorBuffer: colorBuffer,
                                   depthBuffer: depthBuffer,
                                   pointCloud: pointCloud,
                                   depthCameraCalibrationData: depthCalibrationData,
                                   viewMatrix: matrix_identity_float4x4,
                                   into: _metalLayer,
                                   flipsInputHorizontally: false)
        
        if _scanning {
            _bplyDepthDataAccumulator!.accumulate(colorBuffer: colorBuffer,
                                                  colorTime: colorTime,
                                                  depthBuffer: depthBuffer,
                                                  depthTime: depthTime,
                                                  calibrationData: depthCalibrationData)
        }
    }
    
    // MARK: - UI State Management
    
    private var _bplyDepthDataAccumulator: BPLYDepthDataAccumulator?
    
    private var _scanning: Bool = false {
        didSet { _updateUI() }
    }
    
    private var _tapToStartStop: Bool {
        return UserDefaults.standard.bool(forKey: "tap_to_start_stop")
    }
    
    private var _scanDurationSeconds: Int = 5 {
        didSet { _updateUI() }
    }
    
    private var _elapsedSeconds: Int = 0 {
        didSet { _updateUI() }
    }
    
    private var _countdownSeconds: Int = 0 {
        didSet { _updateUI() }
    }
    
    private func _updateUI() {
        // Make sure the view is loaded first
        _ = self.view
        
        scanDurationContainerView.isHidden = _tapToStartStop
        elapsedDurationLabel.isHidden = !_scanning
        scanDurationLabel.text = "\(_scanDurationSeconds) sec"
        elapsedDurationLabel.text = "\(_elapsedSeconds + 1)"
        countdownLabel.isHidden = _countdownSeconds == 0
        countdownLabel.text = "\(_countdownSeconds)"
        shutterButton.setImage(UIImage(named: _scanning ? "CameraButtonRecording" : "CameraButton"), for: UIControl.State.normal)
        shutterButton.isSelected = _countdownSeconds > 0
        _cameraManager.isFocusLocked = _scanning
    }
    
    private func _startCountdown(_ completion: @escaping () -> Void) {
        _countdownSeconds = 3
        _iterateCountdown(completion)
    }
    
    private func _cancelCountdown() {
        AudioAndHapticEngine.shared.scanningCanceled()
        countdownLabel.alpha = 1
        _countdownSeconds = 0
    }
    
    private func _iterateCountdown(_ completion: @escaping () -> Void) {
        AudioAndHapticEngine.shared.countdownCountedDown()
        
        if _countdownSeconds == 0 {
            completion()
            return
        }
        
        countdownLabel.alpha = 1
        UIView.animate(withDuration: 0.7, animations: {
            self.countdownLabel.alpha = 0
        }, completion: { finished in
            if finished && self._countdownSeconds > 0 {
                self._countdownSeconds -= 1
                self._iterateCountdown(completion)
            }
        })
    }
    
    private func _startScanning() {
        AudioAndHapticEngine.shared.scanningBegan()
        _bplyDepthDataAccumulator = BPLYDepthDataAccumulator()
        
        _scanningTimer = Timer.init(timeInterval: 1, repeats: true) { [unowned self] timer in
            self._elapsedSeconds += 1
            
            if !self._tapToStartStop && self._elapsedSeconds >= self._scanDurationSeconds {
                self._stopScanning(reason: .finished)
            }
        }
        RunLoop.current.add(_scanningTimer!, forMode: RunLoop.Mode.default)
        
        _elapsedSeconds = 0
        _scanning = true
    }
    
    private func _stopScanning(reason: ScanningTerminationReason) {
        guard _scanning else { return }
        
        let accumulator = _bplyDepthDataAccumulator!
        _bplyDepthDataAccumulator = nil
        _scanning = false
        _scanningTimer?.invalidate()
        _scanningTimer = nil
        _elapsedSeconds = 0
        _updateUI()
        
        switch reason {
        case .canceled: AudioAndHapticEngine.shared.scanningCanceled()
        case .finished: AudioAndHapticEngine.shared.scanningFinished()
        }
        
        if reason == .finished {
            let zipURL = accumulator.exportFrameSequenceToZip()
            let controller = UIActivityViewController(activityItems: [zipURL], applicationActivities: nil)
            controller.popoverPresentationController?.sourceView = elapsedDurationLabel
            present(controller, animated: true, completion: nil)
        }
    }
    
}
