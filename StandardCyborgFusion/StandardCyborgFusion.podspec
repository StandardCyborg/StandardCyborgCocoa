Pod::Spec.new do |s|
  s.name                    = 'StandardCyborgFusion'
  s.version                 = '2.1.0'
  s.summary                 = 'A framework that performs real time 3D reconstruction using the TrueDepth camera'
  s.homepage                = 'https://github.com/StandardCyborg/StandardCyborgCocoa'
  s.social_media_url        = 'https://twitter.com/StandardCyborg'
  s.documentation_url       = 'https://standardcyborg.com/docs/cocoa-api'
  s.license                 = { :type => 'Commercial', :file => 'LICENSE' }
  s.author                  = { 'Standard Cyborg' => 'sdk@standardcyborg.com' }
  s.source                  = { :http => "https://github.com/StandardCyborg/StandardCyborgCocoa/releases/download/v#{s.version}-StandardCyborgFusion/StandardCyborgFusion.framework.zip" }

  s.ios.deployment_target   = '12.0'
  s.ios.vendored_frameworks = 'ios/StandardCyborgFusion.framework'
  s.ios.xcconfig = {
    'HEADER_SEARCH_PATHS' => '"${PODS_ROOT}/StandardCyborgFusion/ios/StandardCyborgFusion.framework/CppHeaders"'
  }

  s.osx.deployment_target   = '10.14'
  s.osx.vendored_frameworks = 'osx/StandardCyborgFusion.framework'
  s.osx.xcconfig = {
    'HEADER_SEARCH_PATHS' => '"${PODS_ROOT}/StandardCyborgFusion/osx/StandardCyborgFusion.framework/Versions/Current/CppHeaders"'
  }
  # The primary purpose of the Mac OS SDK is to develop command line applications for testing. If
  # the framework is not in ~/Library/Frameworks though, it won't run, period. So this step, though
  # not the most pleasant, sets up a script to install (or replace) the framework locally so that
  # you may run local applications.
  s.osx.script_phase = {
    :name => 'Copy Framework to Library path',
    :execution_position => :before_compile,
    :script => <<-CMD
      mkdir -p "$HOME/Library/Frameworks"
      rm -rf "$HOME/Library/Frameworks/StandardCyborgFusion.framework"
      cp -R "$PODS_ROOT/StandardCyborgFusion/osx/StandardCyborgFusion.framework" "$HOME/Library/Frameworks/"
    CMD
  }
end

