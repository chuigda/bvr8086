#ifndef BVR_LINEAR_ALGEBRA_H
#define BVR_LINEAR_ALGEBRA_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { float x, y; } BvrVec2f;
typedef struct { float x, y, z; } BvrVec3f;
typedef struct { float x, y, z, w; } BvrVec4f;

typedef struct { float r, g, b; } BvrColor3f;
typedef struct { float r, g, b, a; } BvrColor4f;

__attribute__((always_inline)) static inline
BvrVec3f BvrVec4To3(BvrVec4f v) {
  return (BvrVec3f){ v.x, v.y, v.z };
}

__attribute__((always_inline)) static inline
BvrVec2f BvrVec4To2(BvrVec4f v) {
  return (BvrVec2f){ v.x, v.y };
}

__attribute__((always_inline)) static inline
BvrVec2f BvrVec3To2(BvrVec3f v) {
  return (BvrVec2f){ v.x, v.y };
}

__attribute__((always_inline)) static inline
BvrVec2f BvrVecAdd2f(BvrVec2f a, BvrVec2f b) {
  return (BvrVec2f){ a.x + b.x, a.y + b.y };
}

__attribute__((always_inline)) static inline
BvrVec3f BvrVecAdd3f(BvrVec3f a, BvrVec3f b) {
  return (BvrVec3f){ a.x + b.x, a.y + b.y, a.z + b.z };
}

__attribute__((always_inline)) static inline
BvrVec4f BvrVecAdd4f(BvrVec4f a, BvrVec4f b) {
  return (BvrVec4f){ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

__attribute__((always_inline)) static inline
BvrVec2f BvrVecSub2f(BvrVec2f a, BvrVec2f b) {
  return (BvrVec2f){ a.x - b.x, a.y - b.y };
}

__attribute__((always_inline)) static inline
BvrVec3f BvrVecSub3f(BvrVec3f a, BvrVec3f b) {
  return (BvrVec3f){ a.x - b.x, a.y - b.y, a.z - b.z };
}

__attribute__((always_inline)) static inline
BvrVec4f BvrVecSub4f(BvrVec4f a, BvrVec4f b) {
  return (BvrVec4f){ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

__attribute__((always_inline)) static inline
BvrVec2f BvrVecMul2f(BvrVec2f a, float f) {
  return (BvrVec2f){ a.x * f, a.y * f };
}

__attribute__((always_inline)) static inline
BvrVec3f BvrVecMul3f(BvrVec3f a, float f) {
  return (BvrVec3f){ a.x * f, a.y * f, a.z * f };
}

__attribute__((always_inline)) static inline
BvrVec4f BvrVecMul4f(BvrVec4f a, float f) {
  return (BvrVec4f){ a.x * f, a.y * f, a.z * f, a.w * f };
}

__attribute__((always_inline)) static inline
BvrVec2f BvrVecDot2f(BvrVec2f a, BvrVec2f b) {
  return (BvrVec2f){ a.x * b.x, a.y * b.y };
}

__attribute__((always_inline)) static inline
BvrVec3f BvrVecDot3f(BvrVec3f a, BvrVec3f b) {
  return (BvrVec3f){ a.x * b.x, a.y * b.y, a.z * b.z };
}

__attribute__((always_inline)) static inline
BvrVec4f BvrVecDot4f(BvrVec4f a, BvrVec4f b) {
  return (BvrVec4f){ a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

__attribute__((always_inline)) static inline
BvrVec3f BvrVecCross3f(BvrVec3f a, BvrVec3f b) {
  return (BvrVec3f){ a.y * b.z - a.z * b.y,
                     a.z * b.x - a.x * b.z,
                     a.x * b.y - a.y * b.x };
}

__attribute__((always_inline)) static inline
BvrVec3f BvrVecCross4f(BvrVec4f a, BvrVec4f b) {
  return (BvrVec3f){ a.y * b.z - a.z * b.y,
                     a.z * b.x - a.x * b.z,
                     a.x * b.y - a.y * b.x };
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* BVR_LINEAR_ALGEBRA_H */
