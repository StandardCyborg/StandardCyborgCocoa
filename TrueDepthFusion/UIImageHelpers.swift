//
//  UIImageHelpers.swift
//  Capture
//
//  Created by Aaron Thompson on 11/23/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import UIKit

extension UIImage {
    
    func resized(toWidth width: CGFloat) -> UIImage {
        let scale = width / size.width
        let newSize = CGSize(width: width, height: size.height * scale)
        UIGraphicsBeginImageContext(newSize)
        
        draw(in: CGRect(origin: .zero, size: newSize))
        
        let resized = UIGraphicsGetImageFromCurrentImageContext()
        
        UIGraphicsEndImageContext()
        
        return resized!
    }
    
}
