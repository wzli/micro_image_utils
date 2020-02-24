// TODO link to reference paper

#if !defined IN_TYPE || !defined OUT_TYPE
#error "define IN_TYPE and OUT_TYPE macro before including this header"
#endif

#ifndef MIN
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef PARABOLA_INTERSECTION
#define PARABOLA_INTERSECTION(X2, Y2, X1, Y1) \
    (((X2) - (X1)) * ((X2) + (X1) + 1) + (Y2) - (Y1)) / (2 * ((X2) - (X1)))
#endif

#ifndef HOP_DATA
#define HOP_DATA(ARRAY, INDEX, stride) (ARRAY)[(INDEX) * (stride)]
#endif

#define TEMPLATE_CAT3(A, B, C) A##_##B##_##C
#define TEMPLATE(A, B, C) TEMPLATE_CAT3(A, B, C)

typedef void (*TEMPLATE(Transform1D, IN_TYPE, OUT_TYPE))(
        OUT_TYPE* dst, const IN_TYPE* src, int len, int stride);

void TEMPLATE(seperable_2d_transform, IN_TYPE, OUT_TYPE)(OUT_TYPE* dst, const IN_TYPE* src,
        int x_len, int y_len, TEMPLATE(Transform1D, IN_TYPE, OUT_TYPE) transform_1d,
        int line_buffer) {
    dst += x_len * line_buffer;
    for (int16_t col = 0; col < x_len; ++col) {
        transform_1d(dst + col, src + col, y_len, x_len);
    }
    for (int16_t row = 0; row < y_len; ++row, dst += x_len) {
        transform_1d(dst - (x_len * line_buffer), dst, x_len, 1);
    }
}

void TEMPLATE(l1_distance_transform, IN_TYPE, OUT_TYPE)(
        OUT_TYPE* dst, const IN_TYPE* src, int len, int stride) {
    dst[0] = (OUT_TYPE) src[0];
    for (int i = 1; i < len; ++i) {
        HOP_DATA(dst, i, stride) =
                MIN(HOP_DATA(dst, i - 1, stride) + 1, (OUT_TYPE) HOP_DATA(src, i, stride));
    }
    for (int i = len - 2; i >= 0; --i) {
        HOP_DATA(dst, i, stride) = MIN(HOP_DATA(dst, i + 1, stride) + 1, HOP_DATA(dst, i, stride));
    }
}

void TEMPLATE(square_distance_transform, IN_TYPE, OUT_TYPE)(
        OUT_TYPE* dst, const IN_TYPE* src, int len, int stride) {
    HOP_DATA(dst, 0, stride) = 0;
    int envelope_i = 0;
    int envelope_start = 0;
    for (int x = 1; x < len; ++x) {
        for (;;) {
            int envelope_x = (int) HOP_DATA(dst, envelope_i, stride);
            int intersection = (int) PARABOLA_INTERSECTION((IN_TYPE) x, HOP_DATA(src, x, stride),
                    (IN_TYPE) envelope_x, HOP_DATA(src, envelope_x, stride));
            if (envelope_start < intersection || envelope_i == 0) {
                HOP_DATA(dst, ++envelope_i, stride) = (OUT_TYPE) x;
                envelope_start = intersection;
                break;
            }
            if (!--envelope_i) {
                envelope_start = 0;
                continue;
            };
            envelope_x = (int) HOP_DATA(dst, envelope_i, stride);
            int prev_envelope_x = (int) HOP_DATA(dst, envelope_i - 1, stride);
            envelope_start = (int) PARABOLA_INTERSECTION((IN_TYPE) envelope_x,
                    HOP_DATA(src, envelope_x, stride), (IN_TYPE) prev_envelope_x,
                    HOP_DATA(src, prev_envelope_x, stride));
        }
    }
    envelope_start = CLAMP(envelope_start, 0, len - 1);
    int envelope_end = len - 1;
    for (; envelope_end > envelope_start; --envelope_end) {
        HOP_DATA(dst, envelope_end, stride) = HOP_DATA(dst, envelope_i, stride);
    }
    while (--envelope_i) {
        int envelope_x = (int) HOP_DATA(dst, envelope_i, stride);
        int prev_envelope_x = (int) HOP_DATA(dst, envelope_i - 1, stride);
        envelope_start =
                PARABOLA_INTERSECTION((IN_TYPE) envelope_x, HOP_DATA(src, envelope_x, stride),
                        (IN_TYPE) prev_envelope_x, HOP_DATA(src, prev_envelope_x, stride));
        envelope_start = CLAMP(envelope_start, 0, len - 1);
        for (int x = envelope_start; x < envelope_end; ++x) {
            HOP_DATA(dst, x + 1, stride) = HOP_DATA(dst, envelope_i, stride);
        }
        envelope_end = envelope_start;
    }
    while (envelope_start-- > 0) {
        HOP_DATA(dst, envelope_start + 1, stride) = HOP_DATA(dst, 0, stride);
    }
    for (int x = 0; x < len; ++x) {
        int envelope_x = (int) HOP_DATA(dst, MIN(x, len - 1), stride);
        int dx = x - envelope_x;
        HOP_DATA(dst, x, stride) = (OUT_TYPE)(HOP_DATA(src, envelope_x, stride) + SQR(dx));
    }
}

#undef IN_TYPE
#undef OUT_TYPE
#undef TEMPLATE_CAT3
#undef TEMPLATE
