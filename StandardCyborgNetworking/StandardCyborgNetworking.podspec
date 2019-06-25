Pod::Spec.new do |s|

  s.name              = "StandardCyborgNetworking"
  s.version           = "1.3.1"
  s.summary           = "Classes for interacting with the Standard Cyborg Platform networking APIs"
  s.homepage          = 'https://github.com/StandardCyborg/StandardCyborgCocoa'
  s.social_media_url  = 'https://twitter.com/StandardCyborg'
  s.documentation_url = 'https://standardcyborg.com/docs/cocoa-api'
  s.license           = { :type => 'Apache 2.0', :file => 'LICENSE' }
  s.author            = { 'Standard Cyborg' => 'sdk@standardcyborg.com' }
  s.platform          = :ios, "12.0"
  s.swift_version     = '4.2'
  s.requires_arc      = true
  s.source            = { :git => 'https://github.com/StandardCyborg/StandardCyborgCocoa.git', :tag => "v#{s.version}" }
  s.source_files      = "StandardCyborgNetworking/**/*.{swift}"
  s.dependency          "SSZipArchive"
  s.dependency          "ObjectMapper"
  s.dependency          "PromiseKit", "~> 6.0"
  s.dependency          "PromiseKit/Foundation", "~> 6.0"

end
