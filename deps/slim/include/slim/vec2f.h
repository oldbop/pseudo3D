#ifndef SLIM_VEC2F_H
#define SLIM_VEC2F_H

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef float vec2f[2];

static inline void vec2f_add(const vec2f a, const vec2f b, vec2f dest) {
  dest[0] = a[0] + b[0];
  dest[1] = a[1] + b[1];
}

static inline void vec2f_sub(const vec2f a, const vec2f b, vec2f dest) {
  dest[0] = a[0] - b[0];
  dest[1] = a[1] - b[1];
}

static inline void vec2f_dot(const vec2f a, const vec2f b, float *dest) {
  *dest = a[0] * b[0] + a[1] * b[1];
}

static inline void vec2f_norm(const vec2f a, vec2f dest) {

  float len = sqrtf(a[0] * a[0] + a[1] * a[1]);

  dest[0] = a[0] / len;
  dest[1] = a[1] / len;
}

static inline void vec2f_scale(const vec2f a, const float s, vec2f dest) {
  dest[0] = a[0] * s;
  dest[1] = a[1] * s;
}

#ifdef __cplusplus
}
#endif

#endif
