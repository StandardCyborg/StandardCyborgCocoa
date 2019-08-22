Pod::Spec.new do |s|
  s.name                    = 'StandardCyborgFusion'
  s.version                 = '1.3.12'
  s.summary                 = 'A framework that performs real time 3D reconstruction using the TrueDepth camera'
  s.homepage                = 'https://github.com/StandardCyborg/StandardCyborgCocoa'
  s.social_media_url        = 'https://twitter.com/StandardCyborg'
  s.documentation_url       = 'https://standardcyborg.com/docs/cocoa-api'
  s.license                 = { :type => 'Commercial', :file => 'LICENSE' }
  s.author                  = { 'Standard Cyborg' => 'sdk@standardcyborg.com' }
  s.source                  = { :http => "https://github.com/StandardCyborg/StandardCyborgCocoa/releases/download/v#{s.version}-StandardCyborgFusion/StandardCyborgFusion.framework.zip" }
  s.ios.deployment_target   = '12.0'
  s.ios.vendored_frameworks = 'StandardCyborgFusion.framework'
  s.osx.deployment_target   = '10.14'
  s.osx.vendored_frameworks = 'StandardCyborgFusion.framework'
end

