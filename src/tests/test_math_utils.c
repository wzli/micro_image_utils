#include "test_utils.h"
#include "math_utils.h"
#include "stdbool.h"

static int test_math_constants() {
    test_assert(M_E_F == 2.71828182845904523536f);
    test_assert(M_LOG2E_F == 1.44269504088896340736f);
    test_assert(fabsf(M_LOG10E_F - 0.434294481903251827651f) < 0.000001f);
    test_assert(M_LN2_F == 0.693147180559945309417f);
    test_assert(M_LN10_F == 2.30258509299404568402f);
    test_assert(M_PI_F == 3.14159265358979323846f);
    test_assert(M_PI_2_F == 1.57079632679489661923f);
    test_assert(M_PI_4_F == 0.785398163397448309616f);
    test_assert(M_1_PI_F == 0.318309886183790671538f);
    test_assert(M_2_PI_F == 0.636619772367581343076f);
    test_assert(fabsf(M_2_SQRTPI_F - 1.12837916709551257390f) < 0.000001f);
    test_assert(M_SQRT2_F == 1.41421356237309504880f);
    test_assert(M_SQRT1_2_F == 0.707106781186547524401f);
    return 0;
}

static inline bool compare_inv_sqrt(float x) {
    static const float tolerance = 0.002f;
    return fabsf(inv_sqrtf(x) * sqrtf(x) - 1) < tolerance;
};

static int test_inv_sqrtf() {
    for (float f = 0.0001f; f < 100000; f *= 10) {
        test_assert(compare_inv_sqrt(f));
    }
    return 0;
}

static int test_matrix2f_inverse() {
    Matrix2f mat = {{0, 1, 2, 3}};
    Matrix2f identity = m2f_multiply(mat, m2f_inverse(mat));
    test_assert(identity.a == 1);
    test_assert(identity.b == 0);
    test_assert(identity.c == 0);
    test_assert(identity.d == 1);
    return 0;
}

int test_math_utils() {
    test_run(test_math_constants);
    test_run(test_inv_sqrtf);
    test_run(test_matrix2f_inverse);
    return 0;
}
