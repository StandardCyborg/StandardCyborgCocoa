//
//  PaddedLabel.swift
//  TrueDepthFusion
//
//  Created by Aaron Thompson on 9/26/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import UIKit

class PaddedLabel: UILabel {
    
    @IBInspectable var paddingLeft: CGFloat   = 0
    @IBInspectable var paddingRight: CGFloat  = 0
    @IBInspectable var paddingTop: CGFloat    = 0
    @IBInspectable var paddingBottom: CGFloat = 0
    
    override var intrinsicContentSize: CGSize {
        var size = super.intrinsicContentSize
        
        // Weird bug: sometimes intrinsicContentSize is returning a very wide width (2777777, to be exact)
        // Work around it by using sizeThatFits: instead
        size.width = super.sizeThatFits(CGSize(width: 9999, height: size.height)).width
        
        size.width += paddingLeft + paddingRight
        size.height += paddingTop + paddingBottom
        
        return size
    }
    
}
