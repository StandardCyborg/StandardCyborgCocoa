# Note: this is a hack Podspec designed to reference the local source code in
# the `/scsdk` project directory.  Podspec doesn't otherwise
# normally support "in-source" builds (and with this set-up, `pod install` will
# actually copy source files out of this dir to `/Pods`).
libsc_root = "#{__dir__}"

# SDK Repo 'dependencies' directory, as an Xcode-friendly path
libsc_deps_root = "$(PODS_ROOT)/../CppDependencies"

# pod 'EigenCPPCocoa', :path => '../EigenCPPCocoa.podspec.json'

Pod::Spec.new do |s|
  s.name = "scsdk"
  s.version = "1.0.0"
  s.summary = "Standard Cyborg C++ Library for Data, I/O, Algos, and more"
  s.license = { :type => 'MIT', :file => 'LICENSE' }
  s.author = { 'Standard Cyborg' => 'sdk@standardcyborg.com' }
  s.homepage = "https://github.com/StandardCyborg/StandardCyborgCocoa"
  # s.ios.deployment_target = '13.0'
  # s.osx.deployment_target = '11.0'

  # s.source = { :path => libsc_root }
  s.source = { :git => 'git@github.com:StandardCyborg/StandardCyborgCocoa.git#aaptho/merge-scsdk' }
  s.source_files = "scsdk/c++/scsdk/**/*.{h,hpp,cc,cpp}"
  s.public_header_files = "scsdk/c++/scsdk/**/*.{hpp,h}"
  s.header_mappings_dir = "scsdk/c++/scsdk"
  
  # Public Dependencies
  s.dependency 'EigenCPPCocoa'

  # Dependencies - For simplicity, we just forward these via
  # compiler settings NB: Eigen needs to be added differently
  # because how of the Eigen source tree is structured
  s.pod_target_xcconfig = {
    'HEADER_SEARCH_PATHS' => 
      ("$(inherited) " +
        "\"$(PODS_ROOT)/EigenCPPCocoa\" " +
        "#{libsc_deps_root}/happly/ " +
        "#{libsc_deps_root}/json/include/ " +
        "#{libsc_deps_root}/tinygltf/ " +
        "#{libsc_deps_root}/stb/ " + 
        "#{libsc_deps_root}/SparseICP/ " +
        "#{libsc_deps_root}/nanoflann/include/ " +
        "#{libsc_deps_root}/libigl/include/ "
      ),
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++17',
    'CLANG_CXX_LIBRARY' => 'libc++',
    'OTHER_CPLUSPLUSFLAGS' => 
      ("$(inherited) " + 
        "-DFMT_HEADER_ONLY=1" +
        "-DHAVE_CONFIG_H=1 " + 
        "-DHAVE_PTHREAD=1 " +
        "-DGUID_LIBUUID "
      ),
  }
  
  s.user_target_xcconfig = {
    'HEADER_SEARCH_PATHS' => 
      ("$(inherited) " +
        "\"$(PODS_ROOT)/EigenCPPCocoa\" " +
        "#{libsc_deps_root}/happly/ " +
        "#{libsc_deps_root}/json/include/ " +
        "#{libsc_deps_root}/tinygltf/ " +
        "#{libsc_deps_root}/stb/ " + 
        "#{libsc_deps_root}/SparseICP/ " +
        "#{libsc_deps_root}/nanoflann/include/ " +
        "#{libsc_deps_root}/libigl/include/ "
      ),
    }

end
