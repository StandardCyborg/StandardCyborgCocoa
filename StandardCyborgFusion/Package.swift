// swift-tools-version: 6.0

import PackageDescription

let package = Package(
    name: "StandardCyborgFusion",
    platforms: [
        .iOS(.v16), .macOS(.v12)
    ],
    products: [
        .library(
            name: "StandardCyborgFusion",
            targets: ["StandardCyborgFusion"]
        ),
    ],
    dependencies: [
        .package(url: "https://github.com/ZipArchive/ZipArchive.git", from: "2.6.0"),
        .package(path: "../scsdk"),
        .package(path: "../CppDependencies/json"),
        .package(path: "../CppDependencies/libigl"),
        .package(path: "../CppDependencies/PoissonRecon"),
    ],
    targets: [
        .target(
            name: "StandardCyborgFusion",
            dependencies: [
                "json",
                "libigl",
                "scsdk",
                "PoissonRecon",
                "ZipArchive",
            ],
            path: "Sources",
            // resources: [
            //     .process("Models/SCEarLandmarking.mlmodel"),
            //     .process("Models/SCEarTrackingModel.mlmodel"),
            //     .process("Models/SCFootTrackingModel.mlmodel"),
            // ],
            publicHeadersPath: "include",
            cxxSettings: [
                .define("DEBUG", .when(configuration: .debug)),
                .define("IGL_STATIC_LIBRARY"),
                .unsafeFlags(["-fobjc-arc"]),
                .headerSearchPath("."),
                .headerSearchPath("StandardCyborgFusion/Algorithm"),
                .headerSearchPath("StandardCyborgFusion/DataStructures"),
                .headerSearchPath("StandardCyborgFusion/EarLandmarking"),
                .headerSearchPath("StandardCyborgFusion/Helpers"),
                .headerSearchPath("StandardCyborgFusion/IO"),
                .headerSearchPath("StandardCyborgFusion/MetalDepthProcessor"),
                .headerSearchPath("StandardCyborgFusion/Private"),
                .headerSearchPath("include/StandardCyborgFusion"),
            ]
        ),
        .testTarget(
            name: "StandardCyborgFusionTests",
            dependencies: ["StandardCyborgFusion"],
            cxxSettings: [
                .define("DEBUG", .when(configuration: .debug)),
            ]
        )
    ],
    swiftLanguageModes: [.v5],
    cxxLanguageStandard: .cxx17
)
