//
//  ScanningHapticFeedbackEngine.swift
//  Capture
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import AudioToolbox
import Foundation
import UIKit

/** Manages haptic feedback in response to changes in the scanning state */
public class ScanningHapticFeedbackEngine {
    
    public static let shared = ScanningHapticFeedbackEngine()
    
    public init() {
        [
            _hapticImpactMedium,
            _hapticSelection,
            _hapticNotification
        ].forEach { $0.prepare() }
    }
    
    public func countdownCountedDown() {
        _hapticImpactMedium.impactOccurred()
    }
    
    public func scanningBegan() {
        _startScanningTimer()
    }
    
    public func scanningFinished() {
        _stopScanningTimer()
        
        DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + .milliseconds(100)) {
            self._hapticNotification.notificationOccurred(UINotificationFeedbackGenerator.FeedbackType.success)
        }
    }
    
    public func scanningCanceled() {
        _stopScanningTimer()
        
        _hapticNotification.notificationOccurred(UINotificationFeedbackGenerator.FeedbackType.error)
    }
    
    // MARK: - Private
    
    private let _hapticImpactMedium = UIImpactFeedbackGenerator(style: UIImpactFeedbackGenerator.FeedbackStyle.medium)
    private let _hapticSelection = UISelectionFeedbackGenerator()
    private let _hapticNotification = UINotificationFeedbackGenerator()
    
    private let _scanningTimerInterval = 1.0 / 8.0
    private var _scanningTimer: Timer?
    
    private func _startScanningTimer() {
        _scanningTimer = Timer.scheduledTimer(withTimeInterval: _scanningTimerInterval, repeats: true, block: { [weak self] timer in
            self?._hapticSelection.selectionChanged()
        })
    }
    
    private func _stopScanningTimer() {
        _scanningTimer?.invalidate()
        _scanningTimer = nil
    }

}
