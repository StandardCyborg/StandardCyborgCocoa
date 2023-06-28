# StandardCyborgCocoa

## Introduction

This SDK enables real-time 3D scanning and analysis in your own iOS app!

![app-scanning-preview](https://user-images.githubusercontent.com/6288076/51778445-84766880-20b6-11e9-8f46-b63c0a016d8b.png)
![app-scan-preview](https://user-images.githubusercontent.com/6288076/51778444-84766880-20b6-11e9-9f6c-914071556d8e.png)

## Repository Overview

- **StandardCyborgFusion**<br>
  A compiled, binary framework that performs real time 3D reconstruction using the TrueDepth camera. Built from https://github.com/StandardCyborg/StandardCyborgSDK
- **StandardCyborgUI**<br>
  A Swift framework with classes for driving and visualizing scanning using StandardCyborgFusion
- **StandardCyborgExample**<br>
  An iOS app that provides a very simple example of using StandardCyborgUI to perform a reconstruction

## Run included demo app

1. `cd StandardCyborgExample`
1. `pod install`
1. Run on an iOS device with a TrueDepth sensor

## Requirements

- An iOS device with a TrueDepth camera, which is found on iPhone X and later, and on iPad Pro (2018)
- iOS >= 12.0.0

## Feedback, Issues, and Help

## License

This codebase is released under the MIT license

See LICENSE file
