//
//  InitialViewController.swift
//  TrueDepthFusion
//
//  Created by Aaron Thompson on 11/7/18.
//  Copyright © 2018 Standard Cyborg. All rights reserved.
//

import Foundation
import UIKit

class InitialViewController: UIViewController {
    
    @IBOutlet weak var introLabel: UILabel!
    
    override func viewDidLoad() {
        introLabel.text = "Take a Depth Scan with your " + UIDevice.current.localizedModel
        
        if UIDevice.current.userInterfaceIdiom == .pad {
            view.backgroundColor = UIColor(white: 0.9, alpha: 1.0)
        }
    }
    
    @IBAction private func scan(_ sender: UIButton?) {
        let scanToBPLY = UserDefaults.standard.bool(forKey: "dump_raw_frames_to_bply", defaultValue: false)
        let segueIdentifier = scanToBPLY ? "BPLYScanningViewController" : "ScanningViewController"
        performSegue(withIdentifier: segueIdentifier, sender: nil)
    }
    
}

extension UserDefaults {
    
    func bool(forKey key: String, defaultValue: Bool) -> Bool {
        if let defaultNumber = object(forKey: key) as? NSNumber {
            return defaultNumber.boolValue
        } else {
            return defaultValue
        }
    }
    
}
