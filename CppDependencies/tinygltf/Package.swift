// swift-tools-version:6.0
import PackageDescription

let package = Package(
    name: "tinygltf",
    products: [
        .library(name: "tinygltf", targets: ["tinygltf"]),
    ],
    targets: [
        .target(
            name: "tinygltf",
            path: ".",
            publicHeadersPath: "include"
        ),
    ],
    cxxLanguageStandard: .cxx17
)
