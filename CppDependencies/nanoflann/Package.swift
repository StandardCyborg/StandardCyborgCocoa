// swift-tools-version:6.0
import PackageDescription

let package = Package(
    name: "nanoflann",
    products: [
        .library(name: "nanoflann", targets: ["nanoflann"]),
    ],
    targets: [
        .target(
            name: "nanoflann",
            path: ".",
            publicHeadersPath: "include"
        ),
    ],
    cxxLanguageStandard: .cxx17
)
