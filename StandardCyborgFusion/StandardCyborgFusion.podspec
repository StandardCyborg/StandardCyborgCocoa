Pod::Spec.new do |s|
  s.name                    = 'StandardCyborgFusion'
  s.version                 = '2.3.7'
  s.summary                 = 'A framework that performs real time 3D reconstruction using the TrueDepth camera'
  s.homepage                = 'https://github.com/StandardCyborg/StandardCyborgCocoa'
  s.license                 = { :type => 'MIT', :file => 'LICENSE' }
  s.author                  = { 'Standard Cyborg' => 'sdk@standardcyborg.com' }
  s.source                  = { :git => 'git@github.com:StandardCyborg/StandardCyborgCocoa.git', :branch => 'aaptho/merge-scsdk' }
  s.source_files            = 'StandardCyborgFusion/**/*.{h,hpp,c,cpp,m,mm,mlmodel,metal}'

  s.prepare_command = <<-CMD
    if [ -d "CppDependencies/happly/.git" ]; then
      echo "Updating happly repo..."
      cd CppDependencies/happly && git pull origin main && cd -
    else
      echo "Cloning happly repo..."
      git clone git@github.com:StandardCyborg/happly.git CppDependencies/happly
    fi
  
    if [ -d "CppDependencies/json/.git" ]; then
      echo "Updating json repo..."
      cd CppDependencies/json && git pull origin main && cd -
    else
      echo "Cloning json repo..."
      git clone https://github.com/nlohmann/json.git CppDependencies/json
    fi
  
    if [ -d "CppDependencies/tinygltf/.git" ]; then
      echo "Updating tinygltf repo..."
      cd CppDependencies/tinygltf && git pull origin main && cd -
    else
      echo "Cloning tinygltf repo..."
      git clone https://github.com/StandardCyborg/tinygltf.git CppDependencies/tinygltf
    fi
  
    if [ -d "CppDependencies/stb/.git" ]; then
      echo "Updating stb repo..."
      cd CppDependencies/stb && git pull origin main && cd -
    else
      echo "Cloning stb repo..."
      git clone https://github.com/nothings/stb.git CppDependencies/stb
    fi
  
    if [ -d "CppDependencies/SparseICP/.git" ]; then
      echo "Updating SparseICP repo..."
      cd CppDependencies/SparseICP && git pull origin main && cd -
    else
      echo "Cloning SparseICP repo..."
      git clone https://github.com/StandardCyborg/SparseICP.git CppDependencies/SparseICP
    fi
  
    if [ -d "CppDependencies/nanoflann/.git" ]; then
      echo "Updating nanoflann repo..."
      cd CppDependencies/nanoflann && git pull origin main && cd -
    else
      echo "Cloning nanoflann repo..."
      git clone https://github.com/jlblancoc/nanoflann.git CppDependencies/nanoflann
    fi
  
    if [ -d "CppDependencies/libigl/.git" ]; then
      echo "Updating libigl repo..."
      cd CppDependencies/libigl && git pull origin main && cd -
    else
      echo "Cloning libigl repo..."
      git clone https://github.com/StandardCyborg/libigl.git CppDependencies/libigl
    fi
  CMD

  s.subspec 'happly' do |ss|
    ss.source_files = 'CppDependencies/happly.h'
  end
  s.subspec 'json' do |ss|
    ss.source_files = 'CppDependencies/json/single_include/nlohmann/json.hpp'
    ss.preserve_paths = 'single_include'
  end
  s.subspec 'tinygltf' do |ss|
    ss.source_files = 'CppDependencies/tinygltf/tiny_gltf.h'
  end
  s.subspec 'stb' do |ss|
    ss.source_files = 'CppDependencies/stb/*.h'
  end
  s.subspec 'SparseICP' do |ss|
    ss.source_files = 'CppDependencies/SparseICP/SparseICP.h'
  end
  s.subspec 'nanoflann' do |ss|
    ss.source_files = 'CppDependencies/nanoflann/include/nanoflann.hpp'
  end
  
  s.subspec 'scsdk' do |ss|
    ss.ios.deployment_target = '13.0'
    ss.osx.deployment_target = '11.0'
    ss.source_files = 'scsdk/c++/scsdk/**/*.{h,hpp,cc,cpp}'
    ss.public_header_files = 'scsdk/c++/scsdk/**/*.{hpp,h}'
    ss.header_mappings_dir = 'scsdk/c++/scsdk'
    # Public Dependencies
    ss.dependency 'EigenCPPCocoa', '~> 3.4.0'

    ss.pod_target_xcconfig = {
      'HEADER_SEARCH_PATHS' => 
        ("$(inherited) " +
          # "\"$(PODS_ROOT)/EigenCPPCocoa\" " +
          "CppDependencies/happly/ " +
          "CppDependencies/json/include/ " +
          "CppDependencies/tinygltf/ " +
          "CppDependencies/stb/ " + 
          "CppDependencies/SparseICP/ " +
          "CppDependencies/nanoflann/include/ " +
          "CppDependencies/libigl/include/ "
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
    
    ss.user_target_xcconfig = {
      'HEADER_SEARCH_PATHS' => 
        ("$(inherited) " +
          # "\"$(PODS_ROOT)/EigenCPPCocoa\" " +
          "CppDependencies/happly/ " +
          "CppDependencies/json/include/ " +
          "CppDependencies/tinygltf/ " +
          "CppDependencies/stb/ " + 
          "CppDependencies/SparseICP/ " +
          "CppDependencies/nanoflann/include/ " +
          "CppDependencies/libigl/include/ "
        ),
      }
  end
  
  s.dependency 'PoissonRecon', '~> 9.011'
  s.dependency 'EigenCPPCocoa', '~> 3.4.0'
  s.dependency 'SSZipArchive'

  s.xcconfig = {
    'HEADER_SEARCH_PATHS' =>
      "${PODS_ROOT}/StandardCyborgFusion/CppDependencies/happly/ " +
      "${PODS_ROOT}/CppDependencies/json " +
      "${PODS_ROOT}/CppDependencies/tinygltf " +
      "${PODS_ROOT}/CppDependencies/stb " +
      "${PODS_ROOT}/CppDependencies/SparseICP " +
      "${PODS_ROOT}/CppDependencies/nanoflann "
  }

  s.ios.deployment_target   = '13.0'
  s.osx.deployment_target   = '11.0'
end
