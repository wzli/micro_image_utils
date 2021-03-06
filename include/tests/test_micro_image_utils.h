#pragma once

int test_generic_utils();
int test_math_utils();
int test_image_utils();

static inline int test_micro_image_utils() {
    return test_generic_utils() || test_math_utils() || test_image_utils();
}
