// swift-tools-version:6.0
import PackageDescription

let package = Package(
    name: "json",
    products: [
        .library(name: "json", targets: ["json"]),
    ],
    targets: [
        .target(
            name: "json",
            path: ".",
            publicHeadersPath: "include"
        ),
    ],
    cxxLanguageStandard: .cxx17
)
