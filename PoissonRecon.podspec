Pod::Spec.new do |s|
  s.name         = "PoissonRecon"
  s.version      = "9.011"
  s.summary      = "Poisson Surface Reconstruction"
  s.description  = "Screened Poisson Surface Reconstruction and Smoothed Signed Distance Reconstruction"
  s.homepage     = "https://github.com/StandardCyborg/PoissonRecon"
  s.license      = { :type => "MIT", :file => "LICENSE" }
  s.author       = { "Standard Cyborg" => "sdk@standardcyborg.com" }
  
  s.ios.deployment_target = "13.0"
  s.osx.deployment_target = "11.0"
  s.source = { :git => "git@github.com:StandardCyborg/PoissonRecon.git", :branch => "xcodeproj" }
  s.source_files  = [
    "PoissonRecon-Xcode/*.{h,m,mm,cpp}",
    "PoissonRecon-Xcode/Meshing/*.{h,m,mm,cpp}",
    "Src/*.{h,inl,cpp}",
    "**/*.{h,inl,cpp}",
  ]
  s.exclude_files = [
    "Src/PoissonRecon.cpp",
    "Src/SSDRecon.cpp",
    "Src/SurfaceTrimmer.cpp",
    "Src/CmdLineParser.*",
    "Src/VoxelCompare.*",
    "Src/EDTInHeat.*",
    "Src/AdaptiveTreeVisualization.*",
    "Src/ImageStitching.*",
    "JPEG/ckconfig.cpp",
  ]
  s.public_header_files = [
    "PoissonRecon-Xcode/Meshing/MeshingOperation.h",
  ]
  s.preserve_paths = [
    "PoissonRecon-Xcode/*",
    "PoissonRecon-Xcode/**/*",
  ]
  s.requires_arc = true
  s.compiler_flags = "$(inherited) -Wno-deprecated-register -Wno-shift-negative-value -Wno-invalid-offsetof -Wno-unused-variable -Wno-comma -Wno-deprecated-declarations"
  s.xcconfig = {
      'HEADER_SEARCH_PATHS' => 
        ("$(inherited) " +
         "\"$(PODS_ROOT)/PoissonRecon\" " +
         "\"$(PODS_ROOT)/PoissonRecon/PoissonRecon-Xcode\" "
        ),
  }
end
