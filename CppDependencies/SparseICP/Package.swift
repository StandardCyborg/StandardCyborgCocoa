// swift-tools-version:6.0
import PackageDescription

let package = Package(
    name: "SparseICP",
    products: [
        .library(name: "SparseICP", targets: ["SparseICP"]),
    ],
    targets: [
        .target(
            name: "SparseICP",
            path: ".",
            publicHeadersPath: "include"
        ),
    ],
    cxxLanguageStandard: .cxx17
)
