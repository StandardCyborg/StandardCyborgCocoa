// swift-tools-version:6.0
import PackageDescription

let package = Package(
    name: "happly",
    products: [
        .library(name: "happly", targets: ["happly"]),
    ],
    targets: [
        .target(
            name: "happly",
            path: ".",
            publicHeadersPath: "include"
        ),
    ],
    cxxLanguageStandard: .cxx17
)
