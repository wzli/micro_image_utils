#pragma once
#include <stdint.h>

#define SGN(X) (((X) > 0) - ((X) < 0))
#define ABS(X) ((X) < 0 ? -(X) : (X))
#define SQR(X) ((X) * (X))
#define CUBE(X) ((x) * (X) * (X))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define CLAMP(X, MIN, MAX) ((X) < (MIN) ? (MIN) : (X) > (MAX) ? (MAX) : (X))
#define IS_SIGNED(Type) ((Type) -1 < 0x7F)
#define IS_POWER_OF_TWO(X) !(X & (X - 1))

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
