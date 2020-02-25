/*
    Algorithm is based on (but modified to work without an intermediate buffer):
    Distance Transforms of Sampled Functions
    P. Felzenszwalb, D. Huttenlocher
    Theory of Computing, Vol. 8, No. 19, September 2012
*/

// helpers functions

int min(int x, int y) {
    return x < y ? x : y;
}

int clamp(int x, int min, int max) {
    return x < min ? min : x > max ? max : x;
}

static int parabola_intersection(int x2, int y2, int x1, int y1) {
    return ((x2 - x1) * (x2 + x1 + 1) + y2 - y1) / (2 * (x2 - x1));
}

// manhattan distance transform

void manhattan_distance_transform_1d(int* dst, const int* src, int len, int stride) {
    dst[0] = src[0];
    for (int i = 1; i < len; ++i) {
        dst[i * stride] = min(dst[(i - 1) * stride] + 1, src[i * stride]);
    }
    for (int i = len - 2; i >= 0; --i) {
        dst[i * stride] = min(dst[(i + 1) * stride] + 1, dst[i * stride]);
    }
}

void manhattan_distance_transform_2d(int* dst, const int* src, int x_len, int y_len) {
    for (int x = 0; x < x_len; ++x) {
        manhattan_distance_transform_1d(dst + x, src + x, y_len, x_len);
    }
    for (int y = 0; y < y_len; ++y, dst += x_len) {
        manhattan_distance_transform_1d(dst, dst, x_len, 1);
    }
}

// square distance transform

void square_distance_transform_1d(int* dst, const int* src, int len, int stride) {
    dst[0] = 0;
    int envelope_i = 0;
    int envelope_start = 0;
    for (int x = 1; x < len; ++x) {
        for (;;) {
            int envelope_x = dst[envelope_i * stride];
            int intersection =
                    parabola_intersection(x, src[x * stride], envelope_x, src[envelope_x * stride]);
            if (envelope_start < intersection || envelope_i == 0) {
                ++envelope_i;
                dst[envelope_i * stride] = x;
                envelope_start = intersection;
                break;
            }
            if (!--envelope_i) {
                envelope_start = 0;
                continue;
            };
            envelope_x = dst[envelope_i * stride];
            int prev_envelope_x = dst[(envelope_i - 1) * stride];
            envelope_start = parabola_intersection(envelope_x, src[envelope_x * stride],
                    prev_envelope_x, src[prev_envelope_x * stride]);
        }
    }
    envelope_start = clamp(envelope_start, 0, len - 1);
    int envelope_end = len - 1;
    for (; envelope_end > envelope_start; --envelope_end) {
        dst[envelope_end * stride] = dst[envelope_i * stride];
    }
    while (--envelope_i > 0) {
        int envelope_x = dst[envelope_i * stride];
        int prev_envelope_x = dst[(envelope_i - 1) * stride];
        envelope_start = parabola_intersection(envelope_x, src[envelope_x * stride],
                prev_envelope_x, src[prev_envelope_x * stride]);
        envelope_start = clamp(envelope_start, 0, len - 1);
        for (int x = envelope_start; x < envelope_end; ++x) {
            dst[(x + 1) * stride] = dst[envelope_i * stride];
        }
        envelope_end = envelope_start;
    }
    while (envelope_start-- > 0) {
        dst[(envelope_start + 1) * stride] = dst[0];
    }
    for (int x = 0; x < len; ++x) {
        int envelope_x = dst[min(x + 1, len - 1) * stride];
        int dx = x - envelope_x;
        dst[x * stride] = src[envelope_x * stride] + (dx * dx);
        dst[x * stride] = min(dst[x * stride], src[x * stride]);
    }
}

void square_distance_transform_2d(int* dst, const int* src, int x_len, int y_len) {
    for (int y = y_len - 1; y >= 0; --y) {
        for (int x = x_len - 1; x >= 0; --x) {
            dst[x + 1 + (y + 1) * (x_len + 1)] = src[x + y * x_len];
        }
    }
    dst += x_len + 1;
    for (int x = 0; x < x_len; ++x, ++dst) {
        square_distance_transform_1d(dst, dst + 1, y_len, x_len + 1);
    }
    dst -= 2 * x_len + 1;
    for (int y = 0; y < y_len; ++y, dst += x_len) {
        square_distance_transform_1d(dst, dst + y + x_len + 1, x_len, 1);
    }
}
