#include "test_utils.h"
#include "math_utils.h"

static int test_matrix2f_inverse() {
    Matrix2f mat = {{0, 1, 2, 3}};
    Matrix2f identity = m2f_multiply(mat, m2f_inverse(mat));
    test_assert(identity.elements[0] == 1);
    test_assert(identity.elements[1] == 0);
    test_assert(identity.elements[2] == 0);
    test_assert(identity.elements[3] == 1);
    return 0;
}

int test_math_utils() {
    test_run(test_matrix2f_inverse);
    return 0;
}
