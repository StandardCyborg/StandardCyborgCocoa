import Foundation

extension Bundle {
    // Any entity using this prop should exist in the same target as the Bundle(for:) argument otherwise its contents
    // won't be packaged correctly.
    public static let scuiBundle = Bundle(for: ShutterButton.self)
    
    public static let scuiResourcesBundle =
        Bundle(url: scuiBundle.resourceURL!.appendingPathComponent("StandardCyborgUI.bundle"))!
}
