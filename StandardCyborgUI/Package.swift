// swift-tools-version:6.0
import PackageDescription

let package = Package(
    name: "StandardCyborgUI",
    platforms: [
        .iOS(.v16)
    ],
    products: [
        .library(
            name: "StandardCyborgUI",
            targets: ["StandardCyborgUI"]
        )
    ],
    dependencies: [
        // .package(url: "https://github.com/StandardCyborg/StandardCyborgFusion.git", from: "1.7.3")
        .package(path: "../../StandardCyborgSDK/StandardCyborgFusion")
    ],
    targets: [
        .target(
            name: "StandardCyborgUI",
            dependencies: [
                "StandardCyborgFusion",
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
        )
    ],
    swiftLanguageModes: [.v5],
    cxxLanguageStandard: .cxx17
)
