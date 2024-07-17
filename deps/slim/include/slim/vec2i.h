#ifndef SLIM_VEC2I_H
#define SLIM_VEC2I_H

#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t vec2i[2];

static inline void vec2i_add(const vec2i a, const vec2i b, vec2i dest) {
  dest[0] = a[0] + b[0];
  dest[1] = a[1] + b[1];
}

static inline void vec2i_sub(const vec2i a, const vec2i b, vec2i dest) {
  dest[0] = a[0] - b[0];
  dest[1] = a[1] - b[1];
}

static inline void vec2i_dot(const vec2i a, const vec2i b, int32_t *dest) {
  *dest = a[0] * b[0] + a[1] * b[1];
}

static inline void vec2i_norm(const vec2i a, vec2i dest) {

  double x = (double) a[0], y = (double) a[1];
  double len = sqrt(x * x + y * y);

  dest[0] = a[0] / len;
  dest[1] = a[1] / len;
}

static inline void vec2i_scale(const vec2i a, const int32_t s, vec2i dest) {
  dest[0] = a[0] * s;
  dest[1] = a[1] * s;
}

#ifdef __cplusplus
}
#endif

#endif
