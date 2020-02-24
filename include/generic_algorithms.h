#pragma once
#include <stdint.h>
#include <stdbool.h>

#define SWAP(X, Y)                                                      \
    do {                                                                \
        uint8_t* swap_x_ptr = (uint8_t*) &(X);                          \
        uint8_t* swap_y_ptr = (uint8_t*) &(Y);                          \
        for (uint16_t swap_idx = 0; swap_idx < sizeof(X); ++swap_idx) { \
            uint8_t swap_tmp = swap_x_ptr[swap_idx];                    \
            swap_x_ptr[swap_idx] = swap_y_ptr[swap_idx];                \
            swap_y_ptr[swap_idx] = swap_tmp;                            \
        }                                                               \
    } while (0)

#define QUICK_SELECT(ARRAY, LEN, K)                             \
    for (int32_t start = 0, end = (LEN);;) {                    \
        int32_t pivot = start;                                  \
        for (int32_t index = start; index < end - 1; index++) { \
            if ((ARRAY)[index] <= (ARRAY)[end - 1]) {           \
                SWAP((ARRAY)[index], (ARRAY)[pivot]);           \
                pivot++;                                        \
            }                                                   \
        }                                                       \
        SWAP((ARRAY)[end - 1], (ARRAY)[pivot]);                 \
        if ((K) == pivot) {                                     \
            break;                                              \
        } else if (pivot > (K)) {                               \
            end = pivot;                                        \
        } else {                                                \
            start = pivot;                                      \
        }                                                       \
    }
