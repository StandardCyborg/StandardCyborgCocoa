//
//  Pybind11Defs.hpp
//  StandardCyborgSDK
//
//  Created by eric on 2020-01-21.
//  Copyright © 2020 Standard Cyborg. All rights reserved.
//

#pragma once

#ifdef PYBIND11_ONLY

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wregister"

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

typedef pybind11::array_t<float, pybind11::array::c_style | pybind11::array::forcecast> NPFloat;
typedef pybind11::array_t<double, pybind11::array::c_style | pybind11::array::forcecast> NPDouble;

typedef pybind11::array_t<int, pybind11::array::c_style | pybind11::array::forcecast> NPInt;

#pragma clang diagnostic pop

#endif
