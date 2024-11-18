//
//  ShutterButton.swift
//  VICIS-Analysis-Demo
//
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import UIKit

@objc public enum ShutterButtonState: Int {
    case `default`
    case countdown
    case scanning
}

/** Clients can customize by setting their own images per state */
@objc public class ShutterButton: UIButton {
    
    // MARK: - UIView
    
    public override init(frame: CGRect) {
        super.init(frame: frame)
        
        _updateButtonImage()
    }
    
    public required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        
        _updateButtonImage()
    }
    
    // MARK: - Public
    
    @objc public var shutterButtonState: ShutterButtonState = .default {
        didSet { _updateButtonImage() }
    }
    
    @objc public func setImage(_ image: UIImage, for shutterButtonState: ShutterButtonState) {
        _imageForState[shutterButtonState] = image
        
        _updateButtonImage()
    }
    
    // MARK: - Private
    
    private var _imageForState: [ShutterButtonState: UIImage] = [
        .default: UIImage(named: "ShutterButton", in: Bundle.scuiResourcesBundle, compatibleWith: nil)!,
        .countdown: UIImage(named: "ShutterButton-Selected", in: Bundle.scuiResourcesBundle, compatibleWith: nil)!,
        .scanning: UIImage(named: "ShutterButton-Recording", in: Bundle.scuiResourcesBundle, compatibleWith: nil)!,
    ]
    
    private func _updateButtonImage() {
        let image = _imageForState[shutterButtonState]
        
        setImage(image, for: UIControl.State.normal)
    }
    
}
