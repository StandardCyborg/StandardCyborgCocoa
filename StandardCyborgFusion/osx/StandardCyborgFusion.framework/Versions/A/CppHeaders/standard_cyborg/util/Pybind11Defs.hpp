/*
Copyright 2020 Standard Cyborg

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


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

typedef pybind11::array_t<uint8_t, pybind11::array::c_style | pybind11::array::forcecast> NPUInt8;

#pragma clang diagnostic pop

#endif
