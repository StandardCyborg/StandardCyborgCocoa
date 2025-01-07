# StandardCyborgCocoa

## Introduction

This SDK enables real-time 3D scanning on iOS using the TrueDepth camera, plus analysis and other tools for generated scans.

![app-scanning-preview](https://user-images.githubusercontent.com/6288076/51778445-84766880-20b6-11e9-8f46-b63c0a016d8b.png)
![app-scan-preview](https://user-images.githubusercontent.com/6288076/51778444-84766880-20b6-11e9-9f6c-914071556d8e.png)

## Installing

Use Swift Package Manager to add these dependencies
![StandardCyborgFusion](git@github.com:StandardCyborg/StandardCyborgCocoa.git)
![StandardCyborgUI](git@github.com:StandardCyborg/StandardCyborgCocoa.git) (optional)

The version of StandardCyborgFusion hosted via Cocoapods is now deprecated and unmaintained.

## Running the included demo app

1. Open `StandardCyborgExample`
2. Run on an iOS device with a TrueDepth sensor (this will not work in the simulator)

## Repository Overview

- **StandardCyborgFusion**<br>
  A framework that performs real time 3D reconstruction using the TrueDepth camera. It also contains ML models for foot and ear bounding box detection, and for ear feature landmarking.
- **StandardCyborgUI**<br>
  A framework with UI classes for driving and visualizing scanning using StandardCyborgFusion
- **StandardCyborgExample**<br>
  An iOS app that provides a very simple example of using StandardCyborgFusion + StandardCyborgUI to perform a 3D reconstruction and display the result
- **CppDependencies**<br>
  C++ package dependencies used by StandardCyborgFusion
- **scsdk**<br>
  A pure C++ framework that provides the core of reconstruction, plus several data structures and algorithms that may be useful for working with generated scans
- **TrueDepthFusion**<br>
  A more thorough iOS app, useful for iterating on StandardCyborgFusion
- **VisualTesterMac**<br>
  A macOS app used for iterating on StandardCyborgFusion with pre-recorded scan data

## Requirements

An iOS device with a TrueDepth camera, which is found on iPhone X and later, and on iPad Pro (2018) and later

## Feedback, Issues, and Help

Standard Cyborg was once a company that developed 3D-printed prosthetics, then later, this software package for 3D scanning and analysis. The company no longer exists, but in its death, it was able to open-source this 3D scanning and analysis framework under the MIT license.

All feature development, maintenance, and support is provided only by open source maintainers.

## License

This codebase is released under the MIT license

See LICENSE file
