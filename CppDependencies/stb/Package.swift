// swift-tools-version:6.0
import PackageDescription

let package = Package(
    name: "stb",
    products: [
        .library(name: "stb", targets: ["stb"]),
    ],
    targets: [
        .target(
            name: "stb",
            path: ".",
            publicHeadersPath: "include"
        ),
    ],
    cxxLanguageStandard: .cxx17
)
