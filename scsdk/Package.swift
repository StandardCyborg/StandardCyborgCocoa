// swift-tools-version: 6.0
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "scsdk",
    platforms: [.iOS(.v16), .macOS(.v12)],
    products: [
        // Products define the executables and libraries a package produces, making them visible to other packages.
        .library(
            name: "scsdk",
            targets: ["standard_cyborg"]
        ),
    ],
    dependencies: [
        // .package(url: "https://github.com/nlohmann/json.git", from: "3.11.3"),
        .package(path: "../CppDependencies/Eigen"),
        .package(path: "../CppDependencies/json"),
        .package(path: "../CppDependencies/happly"),
        .package(path: "../CppDependencies/nanoflann"),
        .package(path: "../CppDependencies/SparseICP"),
        .package(path: "../CppDependencies/PoissonRecon"),
        .package(path: "../CppDependencies/stb"),
        .package(path: "../CppDependencies/tinygltf"),
    ],
    targets: [
        // Targets are the basic building blocks of a package, defining a module or a test suite.
        // Targets can depend on other targets in this package and products from dependencies.
        .target(
            name: "standard_cyborg",
            dependencies: [
                "Eigen", "happly", "json", "nanoflann", "PoissonRecon", "SparseICP", "stb", "tinygltf"
            ],
            path: "Sources/standard_cyborg",
            publicHeadersPath: "include",
            cxxSettings: [
                .define("DEBUG", .when(configuration: .debug)),
                .define("FMT_HEADER_ONLY", to: "1", .when(platforms: [.iOS, .macOS])),
                .define("HAVE_CONFIG_H", to: "1", .when(platforms: [.iOS, .macOS])),
                .define("HAVE_PTHREAD", to: "1", .when(platforms: [.iOS, .macOS])),
                .define("GUID_LIBUUID", .when(platforms: [.iOS, .macOS])),
            ]
        ),
        // .testTarget(
        //     name: "scsdkTests",
        //     dependencies: ["scsdk"]
        // ),
    ],
    cxxLanguageStandard: .cxx17
)
