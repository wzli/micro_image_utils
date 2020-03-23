#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <complex.h>
#include <math.h>

// define math constants

static const float M_E_F = 2.71828182845904523536f;
static const float M_LOG2E_F = 1.44269504088896340736f;
static const float M_LOG10E_F = 0.434294481903251827651f;
static const float M_LN2_F = 0.693147180559945309417f;
static const float M_LN10_F = 2.30258509299404568402f;
static const float M_PI_F = 3.14159265358979323846f;
static const float M_PI_2_F = 1.57079632679489661923f;
static const float M_PI_4_F = 0.785398163397448309616f;
static const float M_1_PI_F = 0.318309886183790671538f;
static const float M_2_PI_F = 0.636619772367581343076f;
static const float M_2_SQRTPI_F = 1.12837916709551257390f;
static const float M_SQRT2_F = 1.41421356237309504880f;
static const float M_SQRT1_2_F = 0.707106781186547524401f;

// define structs

typedef union {
    struct {
        float x;
        float y;
    };
    float complex z;
    float data[2];
} Vector2f;

typedef union {
    struct {
        float a;
        float b;
        float c;
        float d;
    };
    Vector2f rows[2];
    float data[4];
} Matrix2f;

// float operations

static inline float inv_sqrtf(float x) {
    const float x2 = x * 0.5f;
    const float threehalfs = 1.5f;
    union {
        float f;
        uint32_t i;
    } conv = {x};
    conv.i = 0x5f3759df - (conv.i >> 1);
    conv.f *= (threehalfs - (x2 * conv.f * conv.f));
    return conv.f;
}

// vector operations

static inline bool v2f_is_nan(Vector2f vec) {
    return isnan(vec.x) || isnan(vec.y);
}

static inline Vector2f v2f_multiply(Vector2f a, Vector2f b) {
    return (Vector2f){{a.x * b.x, a.y * b.y}};
}

static inline float v2f_dot(Vector2f a, Vector2f b) {
    return a.x * b.x + a.y * b.y;
}

static inline float v2f_cross(Vector2f a, Vector2f b) {
    return a.x * b.y - a.y * b.x;
}

static inline float v2f_norm_l1(Vector2f vec) {
    return fabsf(vec.x) + fabsf(vec.y);
}

static inline float v2f_norm_sqr(Vector2f vec) {
    return v2f_dot(vec, vec);
}

static inline Vector2f v2f_normalize(Vector2f vec) {
    return (Vector2f)(vec.z * inv_sqrtf(v2f_norm_sqr(vec)));
}

static inline float v2f_distance_sqr(Vector2f a, Vector2f b) {
    return v2f_norm_sqr((Vector2f)(b.z - a.z));
}

static inline float v2f_distance(Vector2f a, Vector2f b) {
    return cabsf(b.z - a.z);
}

// matrix operations

static inline Vector2f m2f_transform(Matrix2f mat, Vector2f vec) {
    return (Vector2f){{v2f_dot(mat.rows[0], vec), v2f_dot(mat.rows[1], vec)}};
}

static inline bool m2f_is_zero(Matrix2f mat) {
    return mat.rows[0].z == 0 && mat.rows[1].z == 0;
}

static inline bool m2f_is_nan(Matrix2f mat) {
    return v2f_is_nan(mat.rows[0]) || v2f_is_nan(mat.rows[1]);
}

static inline float m2f_determinant(Matrix2f mat) {
    return v2f_cross(mat.rows[0], mat.rows[1]);
}

static inline Matrix2f m2f_transpose(Matrix2f mat) {
    return (Matrix2f){{mat.a, mat.c, mat.b, mat.d}};
}

static inline Matrix2f m2f_scale(Matrix2f mat, float scale) {
    return (Matrix2f){
            .rows = {(Vector2f)(mat.rows[0].z * scale), (Vector2f)(mat.rows[1].z * scale)}};
}

static inline Matrix2f m2f_inverse(Matrix2f mat) {
    float inv_det = 1.0f / m2f_determinant(mat);
    return (Matrix2f){{
            mat.d * inv_det,
            -mat.b * inv_det,
            -mat.c * inv_det,
            mat.a * inv_det,
    }};
}

static inline Matrix2f m2f_multiply(Matrix2f a, Matrix2f b) {
    Matrix2f b_t = m2f_transpose(b);
    return (Matrix2f){{
            v2f_dot(a.rows[0], b_t.rows[0]),
            v2f_dot(a.rows[0], b_t.rows[1]),
            v2f_dot(a.rows[1], b_t.rows[0]),
            v2f_dot(a.rows[1], b_t.rows[1]),
    }};
}
