// swift-tools-version:6.0
import PackageDescription

let package = Package(
    name: "libigl",
    products: [
        .library(name: "libigl", targets: ["libigl"]),
    ],
    dependencies: [
        .package(path: "../Eigen"),
    ],
    targets: [
        .target(
            name: "libigl",
            dependencies: [
                "Eigen",
            ],
            path: ".",
            publicHeadersPath: "include",
            cxxSettings: [
                .define("DEBUG", .when(configuration: .debug)),
                .define("FMT_HEADER_ONLY", to: "1"),
                .define("HAVE_CONFIG_H", to: "1"),
                .define("HAVE_PTHREAD", to: "1"),
                .define("GUID_LIBUUID"),
                .define("IGL_STATIC_LIBRARY"),
                // .define("LIBIGL_WITH_PNG", to: "1"),
            ]
        ),
    ],
    cxxLanguageStandard: .cxx17
)
