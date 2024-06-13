Pod::Spec.new do |s|
  s.name                    = 'StandardCyborgFusion'
  s.version                 = '2.3.7'
  s.summary                 = 'A framework that performs real time 3D reconstruction using the TrueDepth camera'
  s.homepage                = 'https://github.com/StandardCyborg/StandardCyborgCocoa'
  s.social_media_url        = 'https://twitter.com/StandardCyborg'
  s.documentation_url       = 'https://standardcyborg.com/docs/cocoa-api'
  s.license                 = { :type => 'Commercial', :file => 'LICENSE' }
  s.author                  = { 'Standard Cyborg' => 'sdk@standardcyborg.com' }
  s.source                  = { :http => "https://github.com/StandardCyborg/StandardCyborgCocoa/releases/download/v#{s.version}-StandardCyborgFusion/StandardCyborgFusion.xcframework.tar.gz" }

  s.ios.deployment_target   = '13.0'
  s.ios.vendored_frameworks = 'StandardCyborgFusion.xcframework'
  s.ios.xcconfig = {
    'HEADER_SEARCH_PATHS' => '"${PODS_ROOT}/StandardCyborgFusion/StandardCyborgFusion.xcframework/ios-arm64/StandardCyborgFusion.framework/CppHeaders"',
  }

  s.osx.deployment_target   = '11.0'
  s.osx.vendored_frameworks = 'StandardCyborgFusion.xcframework'
  s.osx.xcconfig = {
    'HEADER_SEARCH_PATHS' => '"${PODS_ROOT}/StandardCyborgFusion/StandardCyborgFusion.xcframework/macos-arm64_x86_64/StandardCyborgFusion.xcframework/Versions/A/CppHeaders"',
  }
end

