import MediaPlayer
import StandardCyborgFusion
import UIKit

class ScanningViewController: UIViewController, CameraManagerDelegate, SCReconstructionManagerDelegate {
    
    private enum ScanningTerminationReason {
        case canceled
        case finished
    }
    
    // MARK: - Outlets and Actions
    
    @IBOutlet private weak var metalContainerView: UIView!
    @IBOutlet private weak var scanDurationContainerView: UIView!
    @IBOutlet private weak var scanDurationLabel: UILabel!
    @IBOutlet private weak var showScansButton: UIButton!
    @IBOutlet private weak var elapsedDurationLabel: UILabel!
    @IBOutlet private weak var shutterButton: UIButton!
    @IBOutlet private weak var countdownLabel: UILabel!
    @IBOutlet private weak var scanFailedLabel: UILabel!
    
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
    
    @IBAction private func done(_ sender: UIButton) {
        _stopScanning(reason: .finished)
        
        presentingViewController?.dismiss(animated: true, completion: nil)
    }
    
    @IBAction private func showLatestScan(_ sender: UIButton) {
        guard let scan = _appDelegate.scans.first else { return }
        
        _scanPreviewViewController.scan = scan
        
        present(_scanPreviewViewController, animated: true, completion: nil)
    }
    
    // MARK: - Properties
    
    private let _appDelegate = UIApplication.shared.delegate! as! AppDelegate
    private let _metalLayer = CAMetalLayer()
    private let _metalDevice = MTLCreateSystemDefaultDevice()!
    private let _cameraManager = CameraManager()
    private let _motionManager = CMMotionManager()
    
    private var _latestViewMatrix = matrix_identity_float4x4
    private var _scanningTimer: Timer?
    
    private let _meshTexturing = SCMeshTexturing()
    private var _frameIndex = 0
    
    private lazy var _algorithmCommandQueue: MTLCommandQueue = _metalDevice.makeCommandQueue()!
    private lazy var _visualizationCommandQueue: MTLCommandQueue = _metalDevice.makeCommandQueue()!
    private lazy var _reconstructionManager = SCReconstructionManager(device: _metalDevice, commandQueue: _algorithmCommandQueue, maxThreadCount: _maxReconstructionThreadCount)
    private lazy var _scanningViewRenderer = ScanningViewRenderer(device: _metalDevice, commandQueue: _visualizationCommandQueue)
    
    // MARK: - UIViewController
    
    override var supportedInterfaceOrientations: UIInterfaceOrientationMask {
        return .portrait
    }
    
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
        _cameraManager.configureCaptureSession(maxColorResolution: 1920, maxDepthResolution: _useFullResolutionDepthFrames ? 640 : 320, maxFramerate: 30)
        _reconstructionManager.delegate = self
        _reconstructionManager.includesColorBuffersInMetadata = true
        
        _algorithmCommandQueue.label = "ScanningViewController._algorithmCommandQueue"
        _visualizationCommandQueue.label = "ScanningViewController._visualizationCommandQueue"
        
        _installVolumeShutterButton()
        
        NotificationCenter.default.addObserver(self, selector: #selector(_thermalStateChanged(notification:)),
                                               name: ProcessInfo.thermalStateDidChangeNotification, object: nil)
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        let latestScan = _appDelegate.scans.first
        showScansButton.setBackgroundImage(latestScan?.thumbnail, for: UIControl.State.normal)
        scanDurationContainerView.isHidden = _tapToStartStop
        
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
                    UIApplication.shared.open(URL(string: UIApplication.openSettingsURLString)!, options: convertToUIApplicationOpenExternalURLOptionsKeyDictionary([:]), completionHandler: nil)
                })
                
                self.present(alertController, animated: true, completion: nil)
            }
        }
        
        _motionManager.startDeviceMotionUpdates(to: OperationQueue.main) { [weak self] (motion: CMDeviceMotion?, error: Error?) in
            guard
                let motion = motion,
                let strongSelf = self,
                strongSelf._scanning
            else { return }
            
            strongSelf._reconstructionManager.accumulateDeviceMotion(motion)
        }
    }
    
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        
        // Prevents auto screen dimming while the app is foreground
        UIApplication.shared.isIdleTimerDisabled = true
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        
        _cameraManager.stopSession()
        _motionManager.stopDeviceMotionUpdates()
        
        UIApplication.shared.isIdleTimerDisabled = true
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
    
    @objc private func _thermalStateChanged(notification: NSNotification) {
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
        let pointCloud: SCPointCloud
        
        if _scanning {
            pointCloud = _reconstructionManager.buildPointCloud()
        } else {
            pointCloud = _reconstructionManager.reconstructSingleDepthBuffer(depthBuffer,
                                                                             colorBuffer: colorBuffer,
                                                                             with: depthCalibrationData,
                                                                             smoothingPoints: !self._useFullResolutionDepthFrames)
        }
        
        _scanningViewRenderer.draw(colorBuffer: colorBuffer,
                                   depthBuffer: depthBuffer,
                                   pointCloud: pointCloud,
                                   depthCameraCalibrationData: depthCalibrationData,
                                   viewMatrix: _scanning ? _latestViewMatrix : matrix_identity_float4x4,
                                   into: _metalLayer,
                                   flipsInputHorizontally: _reconstructionManager.flipsInputHorizontally)
        
        if _scanning {
            _reconstructionManager.accumulate(depthBuffer: depthBuffer,
                                              colorBuffer: colorBuffer,
                                              calibrationData: depthCalibrationData)

        }
    }
    
    // MARK: - SCReconstructionManagerDelegate
    
    func reconstructionManager(_ manager: SCReconstructionManager,
                               didProcessWith metadata: SCAssimilatedFrameMetadata,
                               statistics: SCReconstructionManagerStatistics)
    {
        _latestViewMatrix = metadata.viewMatrix
        
        if metadata.result == .succeeded || metadata.result == .poorTracking {
            _meshTexturing.cameraCalibrationData = manager.latestCameraCalibrationData
            _meshTexturing.cameraCalibrationFrameWidth = manager.latestCameraCalibrationFrameWidth
            _meshTexturing.cameraCalibrationFrameHeight = manager.latestCameraCalibrationFrameHeight
            
            if _frameIndex % 5 == 0 {
                
                _meshTexturing.saveColorBufferForReconstruction(
                    metadata.colorBuffer!.takeUnretainedValue(),
                    withViewMatrix: metadata.viewMatrix,
                    projectionMatrix: metadata.projectionMatrix)
            }
            
            _frameIndex += 1
        }
        
        if _stopScanOnReconFail && metadata.result == .failed {
            let assimilatedTooFewFrames = statistics.succeededCount < self._failedScanShowPreviewMinFrameCount
            
            self._stopScanning(reason: assimilatedTooFewFrames ? .canceled : .finished)
            
            self._showScanFailedMessage()
        }
    }
    
    func reconstructionManager(_ manager: SCReconstructionManager, didEncounterAPIError error: Error) {
        print("Encountered API error: \(error)")
    }
    
    // MARK: - UI State Management
    
    private lazy var _scanPreviewViewController: ScanPreviewViewController = {
        let scanVC = UIStoryboard(name: "Main", bundle: nil).instantiateViewController(withIdentifier: "ScanPreviewViewController") as! ScanPreviewViewController
        
        scanVC.deletionHandler = { [unowned self] in
            if scanVC.scan!.plyPath != nil {
                self._appDelegate.remove(scanVC.scan!)
            }
            self.dismiss(animated: true, completion: nil)
        }
        
        scanVC.doneHandler = { [unowned self, scanVC] in
            // Save the scan
            if scanVC.scan!.plyPath == nil {
                self._appDelegate.add(scanVC.scan!)
            }
            self.dismiss(animated: true, completion: nil)
        }
        
        return scanVC
    }()
    
    private var _scanning: Bool = false {
        didSet { _updateUI() }
    }
    
    private var _tapToStartStop: Bool {
        return UserDefaults.standard.bool(forKey: "tap_to_start_stop")
    }
    
    private var _useFullResolutionDepthFrames: Bool {
        get { return UserDefaults.standard.bool(forKey: "full_resolution_depth_frames", defaultValue: false) }
        set { UserDefaults.standard.set(newValue, forKey: "full_resolution_depth_frames") }
    }
    
    private var _stopScanOnReconFail: Bool {
           get { return UserDefaults.standard.bool(forKey: "stop_scanning_on_reconstruction_failure", defaultValue: true) }
           set { UserDefaults.standard.set(newValue, forKey: "stop_scanning_on_reconstruction_failure") }
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
        UIView.animate(withDuration: 1.0, animations: {
            self.countdownLabel.alpha = 0
        }, completion: { finished in
            if finished && self._countdownSeconds > 0 {
                self._countdownSeconds -= 1
                self._iterateCountdown(completion)
            }
        })
    }
    
    private let _failedScanShowPreviewMinFrameCount = 50
    
    private lazy var _maxReconstructionThreadCount: Int32 = {
        return UIDevice.current.userInterfaceIdiom == .pad ? 4 : 2
    }()
    
    private func _startScanning() {
        AudioAndHapticEngine.shared.scanningBegan()
        
        _scanningTimer = Timer(timeInterval: 1, repeats: true) { [unowned self] timer in
            self._elapsedSeconds += 1
            
            if !self._tapToStartStop && self._elapsedSeconds >= self._scanDurationSeconds {
                self._stopScanning(reason: .finished)
            }
        }
        RunLoop.current.add(_scanningTimer!, forMode: RunLoop.Mode.default)
        
        _elapsedSeconds = 0
        _scanning = true
        _meshTexturing.reset()
        _frameIndex = 0
    }
    
    private func _stopScanning(reason: ScanningTerminationReason) {
        guard _scanning else { return }
        
        _cameraManager.paused = true
        _scanning = false
        _scanningTimer?.invalidate()
        _scanningTimer = nil
        _elapsedSeconds = 0
        _latestViewMatrix = matrix_identity_float4x4
        _updateUI()
        
        switch reason {
        case .canceled: AudioAndHapticEngine.shared.scanningCanceled()
        case .finished: AudioAndHapticEngine.shared.scanningFinished()
        }
        
        if reason == .finished {
            _cameraManager.stopSession()
            
            _reconstructionManager.finalize {
                let pointCloud = self._reconstructionManager.buildPointCloud()
                
                let scan = Scan(pointCloud: pointCloud,
                                thumbnail: nil,
                                meshTexturing: self._meshTexturing)
                
                self._scanPreviewViewController.scan = scan
                self.present(self._scanPreviewViewController, animated: true, completion: nil)
                
                self._reconstructionManager.reset()
                self._cameraManager.paused = false
            }
        } else {
            _reconstructionManager.reset()
            _cameraManager.paused = false
        }
    }
    
    private func _showScanFailedMessage() {
        scanFailedLabel.isHidden = false
        scanFailedLabel.alpha = 1
        
        UIView.animate(withDuration: 0.8, delay: 3.0, options: [], animations: {
            self.scanFailedLabel.alpha = 0
        }, completion: { finished in
            self.scanFailedLabel.isHidden = true
        })
    }
    
    // MARK: - Volume Shutter Button
    
    private func _installVolumeShutterButton() {
        let volumeView = MPVolumeView(frame: CGRect(x: -CGFloat.greatestFiniteMagnitude, y: 0.0, width: 0.0, height: 0.0))
        view.addSubview(volumeView)
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(_volumeChanged(_:)),
                                               name: NSNotification.Name(rawValue: "AVSystemController_SystemVolumeDidChangeNotification"),
                                               object: nil)
        
    }
    
    @objc private func _volumeChanged(_ notification: Notification) {
        if  let userInfo = notification.userInfo,
			let volumeChangeType = userInfo["AVSystemController_AudioVolumeChangeReasonNotificationParameter"] as? String
        {
			if volumeChangeType == "ExplicitVolumeChange" {
				shutterTapped(nil)
            }
        }
    }
    
}

// Helper function inserted by Swift 4.2 migrator.
private func convertToUIApplicationOpenExternalURLOptionsKeyDictionary(_ input: [String: Any]) -> [UIApplication.OpenExternalURLOptionsKey: Any] {
    return Dictionary(uniqueKeysWithValues: input.map { key, value in (UIApplication.OpenExternalURLOptionsKey(rawValue: key), value)})
}
