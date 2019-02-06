import AVFoundation
import StandardCyborgFusion
import UIKit

@objc public protocol ScanningViewControllerDelegate: AnyObject {
    func scanningViewControllerDidCancel(_ controller: ScanningViewController)
    @objc optional func scanningViewController(_ controller: ScanningViewController, didScan pointCloud: SCPointCloud)
}

/**
    Shows a live color + depth camera preview and shutter button.
 
    When the shutter is tapped, performs a customizable 3-second
    countdown, then starts scanning.
 
    When scanning is manually finished, or if it fails,
    reconstructs a 3D point cloud and informs its delegate.
 
    This class does not itself show a preview of the scan.
 
    Rendering can be customized by setting the scanningViewRenderer
    to your own object conforming to that protocol.
 */
@objc public class ScanningViewController: UIViewController,
    CameraManagerDelegate,
    SCReconstructionManagerDelegate
{
    
    // MARK: - Public
    
    @objc public enum ScanningTerminationReason: Int {
        case canceled
        case finished
    }
    
    @objc public weak var delegate: ScanningViewControllerDelegate?
    
    /** Override to drop in your own visualization */
    @objc public lazy var scanningViewRenderer: ScanningViewRenderer =
        DefaultScanningViewRenderer(device: _metalDevice, commandQueue: _visualizationCommandQueue)
    
    /** The duration of each count in the pre-scan countdown after tapping the shutter button */
    @objc public var countdownPerSecondDuration = 0.75
    
    /** The count of the pre-scan countdown after tapping the shutter button. Set to 0 to disable the countdown. */
    @objc public var countdownStartCount = 3
    
    /** You may customize the dismiss button by setting its public properties, or by hiding it and adding your own */
    @objc public let dismissButton = UIButton()
    
    /** You may customize the shutter button by setting ShutterButton's public properties, or by hiding it and adding your own */
    @objc public let shutterButton = ShutterButton()
    
    /** A convenience initializer that simply calls init() and sets the delegate */
    @objc public convenience init(delegate: ScanningViewControllerDelegate) {
        self.init()
        self.delegate = delegate
    }
    
    @objc public func shutterTapped(_ sender: UIButton?) {
        guard
            presentedViewController == nil,
            _cameraManager.isSessionRunning
            else { return }
        
        switch _state {
        case .default:
            _startCountdown { self.startScanning() }
        case .countdownSeconds(let seconds):
            if seconds > 0 {
                ScanningHapticFeedbackEngine.shared.scanningCanceled()
                _cancelCountdown()
            }
        case .scanning:
            ScanningHapticFeedbackEngine.shared.scanningFinished()
            stopScanning(reason: .finished)
        }
    }
    
    /** Starts scanning immediately */
    @objc public func startScanning() {
        ScanningHapticFeedbackEngine.shared.scanningBegan()
        
        _state = .scanning
    }
    
    /** Stops scanning immediately */
    @objc public func stopScanning(reason: ScanningTerminationReason) {
        guard _state == _State.scanning else { return }
        
        _state = .default
        _latestViewMatrix = matrix_identity_float4x4
        _updateUI()
        
        switch reason {
        case .canceled: ScanningHapticFeedbackEngine.shared.scanningCanceled()
        case .finished: ScanningHapticFeedbackEngine.shared.scanningFinished()
        }
        
        if reason == .finished {
            _cameraManager.stopSession()
            
            // Do final cleanup on the scan
            _reconstructionManager.finalize {
                let pointCloud = self._reconstructionManager.buildPointCloud()
                
                // Reset it now to keep peak memory usage down
                self._reconstructionManager.reset()
                
                self.delegate?.scanningViewController?(self, didScan: pointCloud)
            }
        } else {
            _reconstructionManager.reset()
        }
    }
    
    /** To manually pause the camera output, set this to true */
    @objc public var isCameraPaused: Bool = false {
        didSet {
            if isCameraPaused {
                _cameraManager.stopSession()
            } else {
                _cameraManager.startSession(nil)
            }
        }
    }
    
    // MARK: - UIViewController
    
    override public func viewDidLoad() {
        super.viewDidLoad()
        
        _setUpSubviews()
        
        _cameraManager.delegate = self
        _cameraManager.configureCaptureSession()
        
        _reconstructionManager.delegate = self
        
        NotificationCenter.default.addObserver(self, selector: #selector(_thermalStateChanged), name: ProcessInfo.thermalStateDidChangeNotification, object: nil)
    }
    
    override public func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        
        guard CameraManager.isDepthCameraAvailable else { return }
        
        _startCameraSession()
    }
    
    override public func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        
        stopScanning(reason: ScanningViewController.ScanningTerminationReason.canceled)
        
        _cameraManager.stopSession()
    }
    
    override public func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
        
        _metalContainerView.frame = view.bounds
        
        CATransaction.begin()
        CATransaction.disableActions()
        _metalLayer.frame = _metalContainerView.bounds
        _metalLayer.drawableSize = CGSize(width:  _metalLayer.frame.width  * _metalLayer.contentsScale,
                                          height: _metalLayer.frame.height * _metalLayer.contentsScale)
        CATransaction.commit()
        
        _countdownLabel.sizeToFit()
        _countdownLabel.center = _metalContainerView.center
        _scanFailedLabel.sizeToFit()
        _scanFailedLabel.center = _metalContainerView.center
        dismissButton.sizeToFit()
        dismissButton.center = CGPoint(x: 20 + 0.5 * dismissButton.frame.width,
                                       y: 10 + 0.5 * dismissButton.frame.height + view.safeAreaInsets.top)
        shutterButton.sizeToFit()
        shutterButton.center = CGPoint(x: view.frame.midX,
                                       y: view.frame.maxY - 5 - 0.5 * shutterButton.frame.size.height - view.safeAreaInsets.bottom)
    }
    
    override public func didReceiveMemoryWarning() {
        print("Received low memory warning; stopping scanning")
        stopScanning(reason: .finished)
    }
    
    // MARK: - Notifications
    
    @objc private func _focusOnTap(_ gesture: UITapGestureRecognizer) {
        // Disallow this while scanning
        guard _state != _State.scanning else { return }
        
        let location = gesture.location(in: view)
        
        _cameraManager.focusOnTap(at: location)
    }
    
    @objc private func _thermalStateChanged(notification: Notification) {
        guard let processInfo = notification.object as? ProcessInfo,
            processInfo.thermalState == .critical
            else { return }
        
        DispatchQueue.main.async(execute: _stopScanningForCriticalThermalState)
    }
    
    // MARK: - CameraManagerDelegate
    
    public func cameraDidOutput(colorBuffer: CVPixelBuffer, depthBuffer: CVPixelBuffer, depthCalibrationData: AVCameraCalibrationData) {
        var isScanning = false
        DispatchQueue.main.sync {
            isScanning = self._state == _State.scanning
        }
        
        let pointCloud: SCPointCloud
        
        if isScanning {
            pointCloud = _reconstructionManager.buildPointCloud()
        } else {
            // When the user is not scanning, render a preview by reconstructing the most recent depth buffer
            // into a point cloud from the current point of view, drawn on top of the RGB camera
            // As the result is never saved and the RGB color not used for visualization, there is no need to
            // pass it the color buffer to build the point cloud
            pointCloud = _reconstructionManager.reconstructSingleDepthBuffer(depthBuffer,
                                                                             colorBuffer: nil,
                                                                             with: depthCalibrationData,
                                                                             smoothingPoints: true)
        }
        
        scanningViewRenderer.draw(colorBuffer: colorBuffer,
                                   pointCloud: pointCloud,
                                   depthCameraCalibrationData: depthCalibrationData,
                                   viewMatrix: _latestViewMatrix,
                                   into: _metalLayer)
        
        if isScanning {
            _reconstructionManager.accumulate(depthBuffer: depthBuffer,
                                              colorBuffer: colorBuffer,
                                              calibrationData: depthCalibrationData)
        }
    }
    
    // MARK: - SCReconstructionManagerDelegate
    
    public func reconstructionManager(_ manager: SCReconstructionManager, didProcessWith metadata: SCAssimilatedFrameMetadata, statistics: SCReconstructionManagerStatistics) {
        guard _state == .scanning else { return }
        
        _latestViewMatrix = metadata.viewMatrix
        
        if metadata.result == .failed {
            let assimilatedTooFewFrames = statistics.succeededCount < self._failedScanShowPreviewMinFrameCount
            
            self.stopScanning(reason: assimilatedTooFewFrames ? .canceled : .finished)
        }
    }
    
    public func reconstructionManager(_ manager: SCReconstructionManager, didEncounterAPIError error: Error) {
        print("SCReconstructionManager hit API error: \(error)")
        stopScanning(reason: ScanningViewController.ScanningTerminationReason.canceled)
    }
    
    // MARK: - Private properties
    
    private lazy var _bundle = Bundle(for: ScanningViewController.self)
    
    private let _metalDevice = MTLCreateSystemDefaultDevice()!
    private lazy var _algorithmCommandQueue = _metalDevice.makeCommandQueue()!
    private lazy var _visualizationCommandQueue = _metalDevice.makeCommandQueue()!
    private lazy var _reconstructionManager = SCReconstructionManager(device: _metalDevice, commandQueue: _algorithmCommandQueue, maxThreadCount: _maxReconstructionThreadCount)
    private let _cameraManager = CameraManager()
    private var _latestViewMatrix = matrix_identity_float4x4
    
    private let _metalContainerView = UIView()
    private let _metalLayer = CAMetalLayer()
    private let _countdownLabel = UILabel()
    private let _scanFailedLabel = UILabel()
    
    // MARK: - UI State Management
    
    private enum _State: Equatable {
        case `default`
        case countdownSeconds(Int)
        case scanning
    }
    
    private var _state = _State.default {
        didSet {
            _updateUI()
            
            // Prevent auto screen dimming/lock while scanning
            UIApplication.shared.isIdleTimerDisabled = _state == _State.scanning
        }
    }
    
    private func _setUpSubviews() {
        view.backgroundColor = UIColor.black
        
        _metalLayer.isOpaque = true
        _metalLayer.contentsScale = UIScreen.main.scale
        _metalLayer.device = _metalDevice
        _metalLayer.pixelFormat = MTLPixelFormat.bgra8Unorm
        _metalLayer.framebufferOnly = false
        
        _metalContainerView.layer.addSublayer(_metalLayer)
        view.addSubview(_metalContainerView)
        view.addSubview(_countdownLabel)
        view.addSubview(_scanFailedLabel)
        view.addSubview(dismissButton)
        view.addSubview(shutterButton)
        
        _countdownLabel.textColor = UIColor.white
        _countdownLabel.textAlignment = NSTextAlignment.center
        _countdownLabel.font = UIFont.systemFont(ofSize: 96, weight: UIFont.Weight.semibold)
        
        _scanFailedLabel.text = "Scan failed!\nMove the device slowly\nand keep the subject still"
        _scanFailedLabel.numberOfLines = 0
        _scanFailedLabel.textAlignment = NSTextAlignment.center
        _scanFailedLabel.font = UIFont.systemFont(ofSize: 24, weight: UIFont.Weight.medium)
        _scanFailedLabel.backgroundColor = UIColor(white: 1.0, alpha: 0.8)
        _scanFailedLabel.isHidden = true
        
        dismissButton.setImage(UIImage(named: "Dismiss", in: _bundle, compatibleWith: nil), for: UIControl.State.normal)
        dismissButton.addTarget(self, action: #selector(dismissTapped(_:)), for: UIControl.Event.touchUpInside)
        shutterButton.addTarget(self, action: #selector(shutterTapped(_:)), for: UIControl.Event.touchUpInside)
        
        view.addGestureRecognizer(UITapGestureRecognizer(target: self, action: #selector(_focusOnTap)))
    }
    
    private func _updateUI() {
        loadViewIfNeeded()
        
        switch _state {
        case .default:
            shutterButton.shutterButtonState = .default
            
        case .countdownSeconds(let seconds):
            shutterButton.shutterButtonState = .countdown
            _countdownLabel.isHidden = seconds == 0
            _countdownLabel.text = "\(seconds)"
            _countdownLabel.sizeToFit()
            
        case .scanning:
            shutterButton.shutterButtonState = .scanning
            
        }
        
        _cameraManager.isFocusLocked = _state == .scanning
    }
    
    private func _startCameraSession() {
        _cameraManager.startSession { result in
            switch result {
            case .success:
                break
            case .configurationFailed:
                print("Configuration failed for an unknown reason")
            case .notAuthorized:
                let message = "To take a 3D scan, go to your privacy settings. Tap Camera and turn on for Capture"
                let alertController = UIAlertController(title: "Camera Access", message: message, preferredStyle: .alert)
                alertController.addAction(UIAlertAction(title: "OK",
                                                        style: .cancel,
                                                        handler: nil))
                alertController.addAction(UIAlertAction(title: "Open Settings",
                                                        style: .`default`)
                { _ in
                    UIApplication.shared.open(URL.init(string: UIApplication.openSettingsURLString)!, options: [:], completionHandler: nil)
                })
                
                self.present(alertController, animated: true)
            }
        }
    }
    
    private func _startCountdown(_ completion: @escaping () -> Void) {
        guard countdownStartCount > 0 else {
            completion()
            return
        }
        
        _state = .countdownSeconds(countdownStartCount)
        _iterateCountdown(completion)
    }
    
    private func _cancelCountdown() {
        ScanningHapticFeedbackEngine.shared.scanningCanceled()
        _countdownLabel.alpha = 1
        _state = .default
    }
    
    private func _iterateCountdown(_ completion: @escaping () -> Void) {
        ScanningHapticFeedbackEngine.shared.countdownCountedDown()
        
        if case let _State.countdownSeconds(seconds) = _state, seconds == 0 {
            completion()
            return
        }
        
        _countdownLabel.alpha = 1
        UIView.animate(withDuration: countdownPerSecondDuration, animations: {
            self._countdownLabel.alpha = 0
        }) { finished in
            if
                finished,
                case let _State.countdownSeconds(seconds) = self._state,
                seconds > 0
            {
                self._state = _State.countdownSeconds(seconds - 1)
                self._iterateCountdown(completion)
            }
        }
    }
    
    private let _failedScanShowPreviewMinFrameCount = 50
    
    private lazy var _maxReconstructionThreadCount: Int32 = {
        // Unfortunately, there's not a good way to get the number of *high-performance*
        // CPU cores on iOS, so we have to hard-code this for now
        return UIDevice.current.userInterfaceIdiom == .pad ? 4 : 2
    }()
    
    private func _showScanFailedMessage() {
        _scanFailedLabel.isHidden = false
        _scanFailedLabel.alpha = 1
        
        UIView.animate(withDuration: 0.8, delay: 3.0, options: [], animations: {
            self._scanFailedLabel.alpha = 0
        }, completion: { finished in
            self._scanFailedLabel.isHidden = true
        })
    }
    
    private func _stopScanningForCriticalThermalState() {
        if _state == _State.scanning {
            self.stopScanning(reason: .finished)
        }
        
        let deviceName = UIDevice.current.userInterfaceIdiom == .pad ? "iPad" : "iPhone"
        let alertController = UIAlertController(title: "\(deviceName) is too hot!",
            message: "Please allow \(deviceName) to cool down and try again",
            preferredStyle: .alert)
        alertController.addAction(UIAlertAction(title: "OK", style: .cancel, handler: nil))
        self.present(alertController, animated: true)
    }
    
    @objc private func dismissTapped(_ sender: UIButton?) {
        delegate?.scanningViewControllerDidCancel(self)
    }
    
}
