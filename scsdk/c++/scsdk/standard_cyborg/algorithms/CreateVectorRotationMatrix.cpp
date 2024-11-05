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
#include "standard_cyborg/math/Mat3x4.hpp"
#include "standard_cyborg/math/Mat3x3.hpp"

#include "standard_cyborg/sc3d/Geometry.hpp"

#include "standard_cyborg/algorithms/CreateVectorRotationMatrix.hpp"

#define EPSILON 0.000001

#define CROSS(dest, v1, v2){                 \
          dest[0] = v1[1] * v2[2] - v1[2] * v2[1]; \
          dest[1] = v1[2] * v2[0] - v1[0] * v2[2]; \
          dest[2] = v1[0] * v2[1] - v1[1] * v2[0];}

#define DOT(v1, v2) (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2])

#define SUB(dest, v1, v2){       \
          dest[0] = v1[0] - v2[0]; \
          dest[1] = v1[1] - v2[1]; \
          dest[2] = v1[2] - v2[2];}

/*
 * A function for creating a rotation matrix that rotates a vector called
 * "from" into another vector called "to".
 * Input : from[3], to[3] which both must be *normalized* non-zero vectors
 * Output: mtx[3][3] -- a 3x3 matrix in colum-major form
 * Authors: Tomas Möller, John Hughes 1999
 * Source:
   https://github.com/erich666/jgt-code/blob/master/Volume_04/Number_4/Moller1999/fromtorot.c
 
 License: MIT.
 */
void fromToRotation(float from[3], float to[3], float mtx[3][3]) {
  float v[3];
  float e, h, f;

  CROSS(v, from, to);
  e = DOT(from, to);
  f = (e < 0)? -e:e;
  if (f > 1.0 - EPSILON)     /* "from" and "to"-vector almost parallel */
  {
    float u[3], v[3]; /* temporary storage vectors */
    float x[3];       /* vector most nearly orthogonal to "from" */
    float c1, c2, c3; /* coefficients for later use */
    int i, j;

    x[0] = (from[0] > 0.0)? from[0] : -from[0];
    x[1] = (from[1] > 0.0)? from[1] : -from[1];
    x[2] = (from[2] > 0.0)? from[2] : -from[2];

    if (x[0] < x[1])
    {
      if (x[0] < x[2])
      {
        x[0] = 1.0; x[1] = x[2] = 0.0;
      }
      else
      {
        x[2] = 1.0; x[0] = x[1] = 0.0;
      }
    }
    else
    {
      if (x[1] < x[2])
      {
        x[1] = 1.0; x[0] = x[2] = 0.0;
      }
      else
      {
        x[2] = 1.0; x[0] = x[1] = 0.0;
      }
    }

    u[0] = x[0] - from[0]; u[1] = x[1] - from[1]; u[2] = x[2] - from[2];
    v[0] = x[0] - to[0];   v[1] = x[1] - to[1];   v[2] = x[2] - to[2];

    c1 = 2.0 / DOT(u, u);
    c2 = 2.0 / DOT(v, v);
    c3 = c1 * c2  * DOT(u, v);

    for (i = 0; i < 3; i++) {
      for (j = 0; j < 3; j++) {
        mtx[i][j] =  - c1 * u[i] * u[j]
                     - c2 * v[i] * v[j]
                     + c3 * v[i] * u[j];
      }
      mtx[i][i] += 1.0;
    }
  }
  else  /* the most common case, unless "from"="to", or "from"=-"to" */
  {
#if 0
    /* unoptimized version - a good compiler will optimize this. */
    /* h = (1.0 - e)/DOT(v, v); old code */
    h = 1.0/(1.0 + e);      /* optimization by Gottfried Chen */
    mtx[0][0] = e + h * v[0] * v[0];
    mtx[0][1] = h * v[0] * v[1] - v[2];
    mtx[0][2] = h * v[0] * v[2] + v[1];

    mtx[1][0] = h * v[0] * v[1] + v[2];
    mtx[1][1] = e + h * v[1] * v[1];
    mtx[1][2] = h * v[1] * v[2] - v[0];

    mtx[2][0] = h * v[0] * v[2] - v[1];
    mtx[2][1] = h * v[1] * v[2] + v[0];
    mtx[2][2] = e + h * v[2] * v[2];
#else
    /* ...otherwise use this hand optimized version (9 mults less) */
    float hvx, hvz, hvxy, hvxz, hvyz;
    /* h = (1.0 - e)/DOT(v, v); old code */
    h = 1.0/(1.0 + e);      /* optimization by Gottfried Chen */
    hvx = h * v[0];
    hvz = h * v[2];
    hvxy = hvx * v[1];
    hvxz = hvx * v[2];
    hvyz = hvz * v[1];
    mtx[0][0] = e + hvx * v[0];
    mtx[0][1] = hvxy - v[2];
    mtx[0][2] = hvxz + v[1];

    mtx[1][0] = hvxy + v[2];
    mtx[1][1] = e + h * v[1] * v[1];
    mtx[1][2] = hvyz - v[0];

    mtx[2][0] = hvxz - v[1];
    mtx[2][1] = hvyz + v[0];
    mtx[2][2] = e + hvz * v[2];
#endif
  }
}

namespace standard_cyborg {

namespace algorithms {


math::Mat3x3 createVectorRotationMatrix(const math::Vec3& v0, const math::Vec3& v1) {
    float from[3] = { v0.x, v0.y, v0.z };
    float to[3] = { v1.x, v1.y, v1.z };
    float mtx[3][3];
    fromToRotation(from, to, mtx);
    
    math::Mat3x3 m;
    
    m.m00 = mtx[0][0];
    m.m01 = mtx[0][1];
    m.m02 = mtx[0][2];
    
    m.m10 = mtx[1][0];
    m.m11 = mtx[1][1];
    m.m12 = mtx[1][2];
    
    m.m20 = mtx[2][0];
    m.m21 = mtx[2][1];
    m.m22 = mtx[2][2];
    
    return m;
}

} // algorithms

} // namespace StandardCyborg
