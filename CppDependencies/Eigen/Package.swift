// swift-tools-version:6.0
import PackageDescription

let package = Package(
    name: "Eigen",
    products: [
        .library(name: "Eigen", targets: ["Eigen"]),
    ],
    targets: [
        .target(
            name: "Eigen",
            path: ".",
            publicHeadersPath: "include"
        ),
    ],
    cxxLanguageStandard: .cxx17
)
