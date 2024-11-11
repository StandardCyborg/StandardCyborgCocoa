source 'https://github.com/CocoaPods/Specs.git'
source 'git@github.com:StandardCyborg/SCCocoaPods.git'

inhibit_all_warnings!

platform :osx, '11.0'

target 'StandardCyborgFusion' do
  platform :ios, '13.0'
  pod 'StandardCyborgFusion', :podspec => 'StandardCyborgFusion/StandardCyborgFusion.podspec'
  # pod 'scsdk', :podspec => 'scsdk/scsdk.podspec'
  pod 'SSZipArchive'
  pod 'PoissonRecon', :podspec => 'PoissonRecon.podspec'
end


# Sadly Cocoapods cant do both osx and ios in same target, so we have to make
# a separate one for OSX.
target 'StandardCyborgFusionOSX' do
  platform :osx, '11.0'
  pod 'StandardCyborgFusion', :podspec => 'StandardCyborgFusion/StandardCyborgFusion.podspec'
  # pod 'scsdk', :podspec => 'scsdk/scsdk.podspec'
  pod 'SSZipArchive'
  pod 'PoissonRecon', :podspec => 'PoissonRecon.podspec'
end

target 'StandardCyborgFusionTests' do
  platform :osx, '11.0'
  pod 'StandardCyborgFusion', :podspec => 'StandardCyborgFusion/StandardCyborgFusion.podspec'
  # pod 'scsdk', :podspec => 'scsdk/scsdk.podspec'
end

target 'TrueDepthFusion' do
  platform :ios, '13.0'
  pod 'StandardCyborgFusion', :podspec => 'StandardCyborgFusion/StandardCyborgFusion.podspec'
end

target 'VisualTesterMac' do
  platform :osx, '11.0'
  # pod 'scsdk', :podspec => 'scsdk/scsdk.podspec'
  pod 'StandardCyborgFusion', :podspec => 'StandardCyborgFusion/StandardCyborgFusion.podspec'
end


post_install do |installer|
  installer.pods_project.targets.each do |target|
    target.build_configurations.each do | configuration |
      configuration.build_settings['OTHER_CFLAGS'] = '-fembed-bitcode -Wno-shorten-64-to-32 -Wno-comma'
      configuration.build_settings['OTHER_CPPFLAGS'] = '-Wno-shorten-64-to-32 -Wno-comma'

      # Suppress warnings about upgrading project to automatically select architectures
      configuration.build_settings.delete 'ARCHS'

      # Update minimum deployment target
      if configuration.build_settings['IPHONEOS_DEPLOYMENT_TARGET'].to_f < 13.0
        configuration.build_settings['IPHONEOS_DEPLOYMENT_TARGET'] = '13.0'
      end
      if configuration.build_settings['MACOSX_DEPLOYMENT_TARGET'].to_f < 11.0
        configuration.build_settings['MACOSX_DEPLOYMENT_TARGET'] = '11.0'
      end
    end
  end
end

