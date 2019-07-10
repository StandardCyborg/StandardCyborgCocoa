import Foundation

extension Bundle {
    // Any entity using this prop should exist in the same target as the Bundle(for:) argument otherwise its contents
    // won't be packaged correctly.
    static let scuiBundle = Bundle(url: Bundle(for: ShutterButton.self).url(forResource: "StandardCyborgUI", withExtension: "bundle")!)!
}
