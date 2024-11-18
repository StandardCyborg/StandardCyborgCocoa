# scsdk - A Highly Portable C++ Library for 3D Perception and More

In the directory `scsdk/`, are the components of the library:

 * `math` - Integral containers and linear algebra for 3D Perception.
 * `sc3d` - Core classes for 3D geometry, imaging, selection, and annotation.
 * `scene_graph` - A portable representation for a complete 3D scene.
 * `algorithms` - A small suite of 3D Perception algorithms that operate
      on scsdk data structures.
 * `io` - SERDES for the above data structures supporting a mix of
      JSON, PLY, GLTF, and other file formats.
 * `util` - Various supporting utilities.

`scsdk_test/` contains GTest-based tests for all of the above components. `test_fixture_data/` contains data used in the tests.

## Development

### Building scsdk

#### CMake

To do an out-of-source build, use the following on Mac or Linux:
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j `nproc`
./scsdk_test
```

(On a Mac, you might need to `alias nproc='sysctl -n hw.logicalcpu'`).
