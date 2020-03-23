#include "test_utils.h"
#include "math_utils.h"
#include "stdbool.h"

static inline bool compare_inv_sqrt(float x) {
    static const float tolerance = 0.002f;
    return fabsf(inv_sqrtf(x) * sqrtf(x) - 1) < tolerance;
}

static int test_inv_sqrtf() {
    for (float f = 0.0001f; f < 100000; f *= 10) {
        test_assert(compare_inv_sqrt(f));
    }
    return 0;
}

static int test_matrix2f_inverse() {
    Matrix2f mat = {{0, 1, 2, 3}};
    Matrix2f identity = m2f_multiply(mat, m2f_inverse(mat));
    test_assert(identity.data[0] == 1);
    test_assert(identity.data[1] == 0);
    test_assert(identity.data[2] == 0);
    test_assert(identity.data[3] == 1);
    return 0;
}

int test_math_utils() {
    test_run(test_inv_sqrtf);
    test_run(test_matrix2f_inverse);
    return 0;
}
