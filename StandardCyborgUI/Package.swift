// swift-tools-version: 6.0

import PackageDescription

let package = Package(
    name: "StandardCyborgUI",
    platforms: [.iOS(.v16)],
    products: [
        .library(
            name: "StandardCyborgUI",
            type: .dynamic,
            targets: ["StandardCyborgUI"]
        ),
    ],
    dependencies: [
        .package(path: ".."),
    ],
    targets: [
        .target(
            name: "StandardCyborgUI",
            dependencies: [
                .product(name: "StandardCyborgFusion", package: "StandardCyborgCocoa"),
            ],
            path: "StandardCyborgUI",
            sources: ["Sources"],
            resources: [
                .process("Resources")
            ],
            linkerSettings: [
                .linkedFramework("ARKit"),
                .linkedFramework("QuartzCore"),
                .linkedFramework("CoreVideo")
            ]
        ),
    ],
    swiftLanguageModes: [.v5]
)
