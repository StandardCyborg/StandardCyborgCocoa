#
#  Be sure to run `pod spec lint StandardCyborgUI.podspec' to ensure this is a
#  valid spec and to remove all comments including this before submitting the spec.
#
#  To learn more about Podspec attributes see http://docs.cocoapods.org/specification.html
#  To see working Podspecs in the CocoaPods repo see https://github.com/CocoaPods/Specs/
#

Pod::Spec.new do |s|

  # ―――  Spec Metadata  ―――――――――――――――――――――――――――――――――――――――――――――――――――――――――― #
  #
  #  These will help people to find your library, and whilst it
  #  can feel like a chore to fill in it's definitely to your advantage. The
  #  summary should be tweet-length, and the description more in depth.
  #

  s.name              = "StandardCyborgUI"
  s.version           = "1.7.3"
  s.summary           = "Classes for driving and visualizing scanning using StandardCyborgFusion"
  s.homepage          = 'https://github.com/StandardCyborg/StandardCyborgCocoa'
  s.social_media_url  = 'https://twitter.com/StandardCyborg'
  s.documentation_url = 'https://standardcyborg.com/docs/cocoa-api'
  s.license           = { :type => 'Apache 2.0', :file => 'LICENSE' }
  s.author            = { 'Standard Cyborg' => 'sdk@standardcyborg.com' }
  s.platform          = :ios, "13.0"
  s.swift_version     = '5.0'
  s.requires_arc      = true
  s.source            = { :git => 'https://github.com/StandardCyborg/StandardCyborgCocoa.git', :tag => "v#{s.version}-StandardCyborgUI" }
  s.source_files      = "StandardCyborgUI/**/*.{h,m,swift,metal}"
  s.resource_bundles  = { "StandardCyborgUI" => ["StandardCyborgUI/**/*.{scn,xcassets,metal}"] }
  s.weak_frameworks   = "StandardCyborgFusion", "QuartzCore", "CoreVideo"
  s.dependency          "StandardCyborgFusion"

end
