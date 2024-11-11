//
//  CameraManager.swift
//  TrueDepthFusion
//
//  Created by Aaron Thompson on 10/15/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import AVFoundation
import Foundation

import StandardCyborgFusion

protocol CameraManagerDelegate: AnyObject {
    func cameraDidOutput(colorBuffer: CVPixelBuffer,
                         colorTime: CMTime,
                         depthBuffer: CVPixelBuffer,
                         depthTime: CMTime,
                         depthCalibrationData: AVCameraCalibrationData)
}

class CameraManager: NSObject, AVCaptureDataOutputSynchronizerDelegate {
    
    enum SessionSetupResult {
        case success
        case notAuthorized
        case configurationFailed
    }
    
    deinit {
        _captureSession.removeObserver(self, forKeyPath: "running", context: &_sessionRunningContext)
    }
    
    weak var delegate: CameraManagerDelegate?
    var isFocusLocked: Bool = false
    
    func configureCaptureSession(maxColorResolution: Int = 1280, maxDepthResolution: Int = 640, maxFramerate: Int = 30) {
        // Check video authorization status, video access is required
        switch AVCaptureDevice.authorizationStatus(for: .video) {
        case .authorized:
            break
        case .notDetermined:
            _sessionQueue.suspend()
            AVCaptureDevice.requestAccess(for: .video, completionHandler: { granted in
                if !granted { self._sessionQueue_setupResult = .notAuthorized }
                self._sessionQueue.resume()
            })
        default:
            _sessionQueue_setupResult = .notAuthorized
        }
        
        _sessionQueue.async {
            if self._sessionQueue_setupResult == .success {
                self._sessionQueue_setupResult = self._sessionQueue_configureSession(maxColorResolution: maxColorResolution,
                                                                                     maxDepthResolution: maxDepthResolution,
                                                                                     maxFramerate: maxFramerate)
            }
        }
    }
    
    func startSession(_ completion: ((SessionSetupResult) -> Void)? = nil) {
        _sessionQueue.async {
            let result = self._sessionQueue_setupResult
            switch result {
            case .success:
                // Only set up observers and start the session running if setup succeeded
                self._addObservers()
                self._renderingEnabled = true
                
                self._captureSession.startRunning()
                self._sessionQueue_isSessionRunning = self._captureSession.isRunning
                
            case .notAuthorized, .configurationFailed:
                break
            }
            
            DispatchQueue.main.async {
                completion?(result)
            }
        }
    }
    
    func stopSession() {
        self._renderingEnabled = false
        
        _sessionQueue.async {
            if self._sessionQueue_setupResult == .success {
                self._captureSession.stopRunning()
                self._sessionQueue_isSessionRunning = self._captureSession.isRunning
            }
        }
    }
    
    func focusOnTap(at location: CGPoint) {
        let locationRect = CGRect(origin: location, size: .zero)
        let deviceRect = _videoDataOutput.metadataOutputRectConverted(fromOutputRect: locationRect)
        
        _focus(with: .autoFocus, exposureMode: .autoExpose, at: deviceRect.origin, monitorSubjectAreaChange: true)
    }
    
    var paused: Bool = false {
        didSet {
            _dataOutputQueue.sync {
                self._renderingEnabled = !paused
            }
        }
    }
    
    // MARK: - Properties
    
    private var _sessionQueue_setupResult: SessionSetupResult = .success
    private let _captureSession = AVCaptureSession()
    private var _sessionQueue_isSessionRunning = false
    private let _sessionQueue = DispatchQueue(label: "Session", qos: DispatchQoS.userInitiated, attributes: [], autoreleaseFrequency: .workItem)
    private let _videoDeviceDiscoverySession = AVCaptureDevice.DiscoverySession(deviceTypes: [.builtInTrueDepthCamera],
                                                                                mediaType: .video,
                                                                                position: .front)
    private var _videoDeviceInput: AVCaptureDeviceInput!
    private let _dataOutputQueue = DispatchQueue(label: "Video data output", qos: .userInitiated, attributes: [], autoreleaseFrequency: .workItem)
    private let _videoDataOutput = AVCaptureVideoDataOutput()
    private let _depthDataOutput = AVCaptureDepthDataOutput()
    private var _outputSynchronizer: AVCaptureDataOutputSynchronizer?
    
    private let _renderingEnabledLock = NSLock()
    private var __renderingEnabled: Bool = false
    private var _renderingEnabled: Bool {
        get {
            _renderingEnabledLock.lock()
            let enabled = __renderingEnabled
            _renderingEnabledLock.unlock()
            return enabled
        }
        set {
            _renderingEnabledLock.lock()
            __renderingEnabled = newValue
            _renderingEnabledLock.unlock()
        }
    }
    
    // MARK: - KVO and Notifications
    
    private var _sessionRunningContext = 0
    
    private func _addObservers() {
        NotificationCenter.default.addObserver(self, selector: #selector(sessionRuntimeError),
                                               name: NSNotification.Name.AVCaptureSessionRuntimeError, object: _captureSession)
        
        _captureSession.addObserver(self, forKeyPath: "running", options: NSKeyValueObservingOptions.new, context: &_sessionRunningContext)
        
        /*
         A session can only run when the app is full screen. It will be interrupted
         in a multi-app layout, introduced in iOS 9, see also the documentation of
         AVCaptureSessionInterruptionReason. Add observers to handle these session
         interruptions and show a preview is paused message. See the documentation
         of AVCaptureSessionWasInterruptedNotification for other interruption reasons.
         */
        NotificationCenter.default.addObserver(self, selector: #selector(didEnterBackground),
                                               name: UIApplication.didEnterBackgroundNotification, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(willEnterForground),
                                               name: UIApplication.willEnterForegroundNotification, object: nil)
        NotificationCenter.default.addObserver(self, selector: #selector(sessionWasInterrupted),
                                               name: NSNotification.Name.AVCaptureSessionWasInterrupted,
                                               object: _captureSession)
        NotificationCenter.default.addObserver(self, selector: #selector(sessionInterruptionEnded),
                                               name: NSNotification.Name.AVCaptureSessionInterruptionEnded,
                                               object: _captureSession)
        NotificationCenter.default.addObserver(self, selector: #selector(subjectAreaDidChange),
                                               name: NSNotification.Name.AVCaptureDeviceSubjectAreaDidChange,
                                               object: _videoDeviceInput.device)
    }
    
    override func observeValue(forKeyPath keyPath: String?, of object: Any?, change: [NSKeyValueChangeKey: Any]?, context: UnsafeMutableRawPointer?) {
        if context != &_sessionRunningContext {
            super.observeValue(forKeyPath: keyPath, of: object, change: change, context: context)
        }
    }
    
    @objc private func sessionWasInterrupted(notification: NSNotification) {
        if let userInfoValue = notification.userInfo?[AVCaptureSessionInterruptionReasonKey] as AnyObject?,
            let reasonIntegerValue = userInfoValue.integerValue,
            let reason = AVCaptureSession.InterruptionReason(rawValue: reasonIntegerValue) {
            print("Capture session was interrupted with reason \(reason)")
        }
    }
    
    @objc private func sessionInterruptionEnded(notification: NSNotification) {
    }
    
    @objc private func sessionRuntimeError(notification: NSNotification) {
        guard let errorValue = notification.userInfo?[AVCaptureSessionErrorKey] as? NSError else {
            return
        }
        
        let error = AVError(_nsError: errorValue)
        print("Capture session runtime error: \(error)")
        
        // Automatically try to restart the session running if media services were reset and the last start running succeeded.
        // Otherwise, enable the user to try to resume the session running.
        if error.code == .mediaServicesWereReset {
            _sessionQueue.async {
                if self._sessionQueue_isSessionRunning {
                    self._captureSession.startRunning()
                    self._sessionQueue_isSessionRunning = self._captureSession.isRunning
                }
            }
        }
    }
    
    @objc private func subjectAreaDidChange(notification: NSNotification) {
        guard !isFocusLocked else { return }
        
        let devicePoint = CGPoint(x: 0.5, y: 0.5)
        _focus(with: .continuousAutoFocus, exposureMode: .continuousAutoExposure, at: devicePoint, monitorSubjectAreaChange: false)
    }
    
    @objc private func didEnterBackground(notification: NSNotification) {
        // Free up resources
        self._renderingEnabled = false
    }
    
    @objc private func willEnterForground(notification: NSNotification) {
        self._renderingEnabled = true
    }
    
    // MARK: - AVCaptureDataOutputSynchronizerDelegate
    
    func dataOutputSynchronizer(_ synchronizer: AVCaptureDataOutputSynchronizer,
                                didOutput synchronizedDataCollection: AVCaptureSynchronizedDataCollection)
    {
        guard _renderingEnabled else { return }
        
        guard
            let syncedDepthData: AVCaptureSynchronizedDepthData =
                    synchronizedDataCollection.synchronizedData(for: _depthDataOutput) as? AVCaptureSynchronizedDepthData,
            let syncedVideoData: AVCaptureSynchronizedSampleBufferData =
                    synchronizedDataCollection.synchronizedData(for: _videoDataOutput) as? AVCaptureSynchronizedSampleBufferData
        else { return /* Only work on synced pairs */ }
        
        guard !syncedDepthData.depthDataWasDropped &&
              !syncedVideoData.sampleBufferWasDropped
        else { return }

        let depthData = syncedDepthData.depthData
        let depthBuffer = depthData.depthDataMap
        let sampleBuffer = syncedVideoData.sampleBuffer
        guard let colorBuffer = CMSampleBufferGetImageBuffer(sampleBuffer),
              let depthCalibrationData = depthData.cameraCalibrationData
        else { return }
        
        let colorTime = syncedVideoData.timestamp
        let depthTime = syncedDepthData.timestamp
        
        delegate?.cameraDidOutput(colorBuffer: colorBuffer,
                                  colorTime: colorTime,
                                  depthBuffer: depthBuffer,
                                  depthTime: depthTime,
                                  depthCalibrationData: depthCalibrationData)
    }
    
    // MARK: - Internal
    
    private func _sessionQueue_configureSession(maxColorResolution: Int,
                                                maxDepthResolution: Int,
                                                maxFramerate: Int) -> SessionSetupResult
    {
        guard let videoDevice = _videoDeviceDiscoverySession.devices.first else {
            print("Could not find any video device")
            return .configurationFailed
        }
        do {
            _videoDeviceInput = try AVCaptureDeviceInput(device: videoDevice)
        } catch {
            print("Could not create video device input: \(error)")
            return .configurationFailed
        }
        guard _captureSession.canAddInput(_videoDeviceInput) else {
            print("Could not add video device input to the session")
            return .configurationFailed
        }
        guard _captureSession.canAddOutput(_videoDataOutput) else {
            print("Could not add video data output to the session")
            return .configurationFailed
        }
        guard _captureSession.canAddOutput(_depthDataOutput) else {
            print("Could not add depth data output to the session")
            return .configurationFailed
        }
        
        _captureSession.beginConfiguration()
        _captureSession.sessionPreset = AVCaptureSession.Preset(maxWidth: maxColorResolution)
        _captureSession.addInput(_videoDeviceInput)
        _captureSession.addOutput(_videoDataOutput)
        _captureSession.addOutput(_depthDataOutput)
        _videoDataOutput.videoSettings = [kCVPixelBufferPixelFormatTypeKey as String: Int(kCVPixelFormatType_32BGRA)]
        _videoDataOutput.alwaysDiscardsLateVideoFrames = true
        _depthDataOutput.isFilteringEnabled = false
        _depthDataOutput.alwaysDiscardsLateDepthData = true
        
        if let captureConnection = _videoDataOutput.connection(with: AVMediaType.video) {
            if captureConnection.isCameraIntrinsicMatrixDeliverySupported {
                captureConnection.isCameraIntrinsicMatrixDeliveryEnabled = true
            }
        }
        
        if let connection = _depthDataOutput.connection(with: .depthData) {
            connection.isEnabled = true
        } else {
            print("No AVCaptureConnection")
        }
        
        // Search for highest resolution with half-point depth values
        let depthFormats = videoDevice.activeFormat.supportedDepthDataFormats
        let selectedFormat = depthFormats.filter {
            CMFormatDescriptionGetMediaSubType($0.formatDescription) == kCVPixelFormatType_DepthFloat32
        }.filter {
             CMVideoFormatDescriptionGetDimensions($0.formatDescription).width <= maxDepthResolution
        }.max { first, second in
                CMVideoFormatDescriptionGetDimensions(first.formatDescription).width
              < CMVideoFormatDescriptionGetDimensions(second.formatDescription).width
        }
        
        do {
            try videoDevice.lockForConfiguration()
            videoDevice.activeDepthDataFormat = selectedFormat
            videoDevice.activeDepthDataMinFrameDuration = CMTimeMake(value: 1, timescale: Int32(maxFramerate))
            // DEV: This seems to be coming in as sRGB, as evidenced by the result of `videoDevice.activeFormat`
            videoDevice.unlockForConfiguration()
        } catch {
            print("Could not lock device for configuration: \(error)")
            _captureSession.commitConfiguration()
            return .configurationFailed
        }
        
        // Use an AVCaptureDataOutputSynchronizer to synchronize the video data and depth data outputs.
        // The first output in the dataOutputs array, in this case the AVCaptureVideoDataOutput, is the "master" output.
        _outputSynchronizer = AVCaptureDataOutputSynchronizer(dataOutputs: [_videoDataOutput, _depthDataOutput])
        _outputSynchronizer!.setDelegate(self, queue: _dataOutputQueue)
        _captureSession.commitConfiguration()
        
        return .success
    }
    
    private func _focus(with focusMode: AVCaptureDevice.FocusMode,
                        exposureMode: AVCaptureDevice.ExposureMode,
                        at devicePoint: CGPoint,
                        monitorSubjectAreaChange: Bool)
    {
        _sessionQueue.async {
            let videoDevice = self._videoDeviceInput.device
            
            do {
                try videoDevice.lockForConfiguration()
                if videoDevice.isFocusPointOfInterestSupported && videoDevice.isFocusModeSupported(focusMode) {
                    videoDevice.focusPointOfInterest = devicePoint
                    videoDevice.focusMode = focusMode
                }
                
                if videoDevice.isExposurePointOfInterestSupported && videoDevice.isExposureModeSupported(exposureMode) {
                    videoDevice.exposurePointOfInterest = devicePoint
                    videoDevice.exposureMode = exposureMode
                }
                
                videoDevice.isSubjectAreaChangeMonitoringEnabled = monitorSubjectAreaChange
                videoDevice.unlockForConfiguration()
            } catch {
                print("Could not lock device for configuration: \(error)")
            }
        }
    }
    
}

fileprivate extension AVCaptureSession.Preset {
    init(maxWidth: Int) {
        switch maxWidth {
        case 1...352:
            self = .cif352x288
        case 353...640:
            self = .vga640x480
        case 641...1280:
            self = .hd1280x720
        case 1281...1920:
            self = .hd1920x1080
        case 1921..<3840:
            self = .hd4K3840x2160
        default:
            self = .photo
        }
    }
}
