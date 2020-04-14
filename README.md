# StandardCyborgCocoa

## Introduction

This SDK enables real-time 3D scanning, asset storage, and analysis in your own iOS app using algorithms and servers from [Standard Cyborg](https://standardcyborg.com).

The quickest way to get started is to clone this repository, then follow the [iOS Quickstart Guide](https://standardcyborg.com/docs/ios-quickstart)

![app-scanning-preview](https://user-images.githubusercontent.com/6288076/51778445-84766880-20b6-11e9-8f46-b63c0a016d8b.png)
![app-scan-preview](https://user-images.githubusercontent.com/6288076/51778444-84766880-20b6-11e9-9f6c-914071556d8e.png)

## Repository Overview

- **StandardCyborgFusion**<br>
  A compiled, binary framework that performs real time 3D reconstruction using the TrueDepth camera
- **StandardCyborgNetworking**<br>
  A Swift framework that connects to the platform.standardcyborg.com API
- **StandardCyborgUI**<br>
  A Swift framework with classes for driving and visualizing scanning using StandardCyborgFusion
- **StandardCyborgExample**<br>
  An iOS app that provides a very simple example of using StandardCyborgUI

## Run included demo app

1. `cd StandardCyborgExample`
2. Drop your API token into `Info.plist` (from your platform.standardcyborg.com account)
3. `pod install`
4. Run on a device with TrueDepth

## Documentation

All documentation is hosted at https://standardcyborg.com/docs/ios-quickstart/

## Requirements

- An iOS device with a TrueDepth camera, which is found on iPhone X and later, and on iPad Pro (2018)
- iOS >= 12.0.0
- Free API keys from https://standardcyborg.com

## Feedback, Issues, and Help

Email sdk@standardcyborg.com and we will gladly reply in a timely manner.

You may also find an answer to your question in our [FAQ](https://standardcyborg.com/docs/faq)

## License

The StandardCyborgFusion framework is provided as a compiled, closed-source framework with a proprietary license.
The remaining projects within this repository are open source with an Apache 2.0 license.
See the LICENSE file within each subdirectory in this project for specific details.
