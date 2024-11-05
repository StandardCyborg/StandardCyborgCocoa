# scsdk

- *c++/* The C++ source code of our Core SDK. It is included in the open source release for documentation purposes, and for
allowing outside developers to fix minor bugs they come across. But the C++ SDK is not the official recommended and supported way of using our SDK


## Development

scsdk currently relies on dependencies through both CocoaPods 
as well as the StandardCyborgSDK repo's more general dependencies directory.
When adding a new dependency, you must update configuration in [scsdk.podspec](scsdk.podspec)

When modifying the root `Podfile`, run this command to update the `Pods/`
directory at the root of the repo.  (You'll also want to `git add Pods/`
to include your changes with your branch).

Pod update command, run from root of repo:
`rm -rf Pods Podfile.lock && pod cache clean --all && pod install --clean-install --verbose --repo-update`
