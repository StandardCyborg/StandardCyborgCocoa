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
        .package(path: "../CppDependencies/PoissonRecon"),
    ],
    targets: [
        .target(
            name: "StandardCyborgFusion",
            dependencies: [
                "json",
                "scsdk",
                "PoissonRecon",
                "ZipArchive",
            ],
            path: "Sources",
            // resources: [
            //     .process("StandardCyborgFusion/Models/SCEarLandmarking.mlmodel"),
            //     .process("StandardCyborgFusion/Models/SCEarTrackingModel.mlmodel"),
            //     .process("StandardCyborgFusion/Models/SCFootTrackingModel.mlmodel"),
            // ],
            publicHeadersPath: "include",
            cxxSettings: [
                .define("DEBUG", .when(configuration: .debug)),
                .unsafeFlags(["-fobjc-arc"]),
                .headerSearchPath("."),
                .headerSearchPath("../libigl/include"),
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
