#include "image_utils.h"
#include <assert.h>
#include <stdio.h>

void img_convolution_filter(ImageMatrix* dst, const ImageMatrix src, const ImageMatrixInt8 kernel) {
    assert(dst && IMG_IS_VALID(*dst) && IMG_IS_VALID(src) && IMG_IS_VALID(kernel));
    IMG_CONVOLUTION(*dst, src, kernel, 1, 0, UINT8_MAX);
}

void img_max_filter(ImageMatrix* dst, const ImageMatrix src, uint16_t block_size) {
    assert(dst && IMG_IS_VALID(*dst) && IMG_IS_VALID(src) && block_size > 0);
    IMG_REDUCE_FILTER(*dst, src, block_size, MAX);
}

void img_min_filter(ImageMatrix* dst, const ImageMatrix src, uint16_t block_size) {
    assert(dst && IMG_IS_VALID(*dst) && IMG_IS_VALID(src) && block_size > 0);
    IMG_REDUCE_FILTER(*dst, src, block_size, MIN);
}

void img_median_filter(ImageMatrix* dst, const ImageMatrix src, ImageMatrix window) {
    assert(dst && IMG_IS_VALID(*dst) && IMG_IS_VALID(src) && IMG_IS_VALID(window));
    IMG_VALID_PADDING(*dst, src, window);
    int16_t middle_index = IMG_PIXEL_COUNT(window) / 2;
    FOR_EACH_PIXEL(*dst) {
        ImagePoint top_left = {col, row};
        IMG_CROP(window, src, top_left);
        QUICK_SELECT(window.data, IMG_PIXEL_COUNT(window), middle_index);
        PIXEL(*dst, row, col) = window.data[middle_index];
    }
}

uint8_t img_nearest_interpolation(const ImageMatrix mat, Vector2f position) {
    assert(IMG_IS_VALID(mat));
    return PIXEL(mat, CLAMP((int16_t) position.xy[1], 0, mat.size.y - 1),
            CLAMP((int16_t) position.xy[0], 0, mat.size.x - 1));
}

uint8_t img_bilinear_interpolation(const ImageMatrix mat, Vector2f position) {
    assert(IMG_IS_VALID(mat));
    int16_t right = position.xy[0] + 0.5f;
    int16_t bottom = position.xy[1] + 0.5f;
    int16_t left = MAX(right - 1, 0);
    int16_t top = MAX(bottom - 1, 0);
    right = MIN(right, mat.size.x - 1);
    bottom = MIN(bottom, mat.size.y - 1);
    Vector2f progress = {{position.xy[0] - 0.5f - left, position.xy[1] - 0.5f - top}};
    float top_average = (float) PIXEL(mat, top, left) +
                        progress.xy[0] * (PIXEL(mat, top, right) - PIXEL(mat, top, left));
    float bottom_average = (float) PIXEL(mat, bottom, left) +
                           progress.xy[0] * (PIXEL(mat, bottom, right) - PIXEL(mat, bottom, left));
    return top_average + progress.xy[1] * (bottom_average - top_average);
}

static inline float cubic_interpolation(float p[4], float x) {
    assert(p);
    return p[1] + 0.5f * x *
                          (p[2] - p[0] +
                                  x * (2.0f * p[0] - 5.0f * p[1] + 4.0f * p[2] - p[3] +
                                              x * (3.0f * (p[1] - p[2]) + p[3] - p[0])));
}

uint8_t img_bicubic_interpolation(const ImageMatrix mat, Vector2f position) {
    assert(IMG_IS_VALID(mat));
    int16_t row0 = position.xy[1] - 1.5f;
    int16_t col0 = position.xy[0] - 1.5f;
    if (row0 < 0 || col0 < 0 || row0 + 4 > mat.size.y || col0 + 4 > mat.size.x) {
        return img_bilinear_interpolation(mat, position);
    }
    float y_points[4];
    for (int16_t i = 0; i < 4; ++i) {
        float x_points[4];
        for (int16_t j = 0; j < 4; ++j) {
            x_points[j] = PIXEL(mat, row0 + i, col0 + j);
        }
        y_points[i] = cubic_interpolation(x_points, position.xy[0] - col0 - 1.5f);
    }
    float result = cubic_interpolation(y_points, position.xy[1] - row0 - 1.5f);
    return CLAMP(result, 0, UINT8_MAX);
}

void img_resize(ImageMatrix dst, const ImageMatrix src, ImageInterpolation interpolation) {
    assert(IMG_IS_VALID(dst) && IMG_IS_VALID(src) && interpolation);
    Vector2f scale = {{(float) src.size.x / dst.size.x, (float) src.size.y / dst.size.y}};
    FOR_EACH_PIXEL(dst) {
        Vector2f position = {{0.5f + col, 0.5f + row}};
        PIXEL(dst, row, col) = interpolation(src, v2f_multiply(position, scale));
    }
}

void img_rotate(ImageMatrix dst, const ImageMatrix src, Vector2f rotation, uint8_t bg_fill,
        ImageInterpolation interpolation) {
    Matrix2f transform = {{rotation.xy[0], -rotation.xy[1], rotation.xy[1], rotation.xy[0]}};
    img_affine_transform(dst, src, transform, bg_fill, interpolation);
}

void img_affine_transform(ImageMatrix dst, const ImageMatrix src, Matrix2f transform,
        uint8_t bg_fill, ImageInterpolation interpolation) {
    assert(IMG_IS_VALID(dst) && IMG_IS_VALID(src) && interpolation);
    assert(!m2f_is_nan(transform) && m2f_determinant(transform) != 0.0f);
    Vector2f src_center = {{0.5f * src.size.x, 0.5f * src.size.y}};
    Vector2f dst_center = {{0.5f * dst.size.x, 0.5f * dst.size.y}};
    transform = m2f_inverse(transform);
    FOR_EACH_PIXEL(dst) {
        Vector2f from_center = {{0.5f + col - dst_center.xy[0], 0.5f + row - dst_center.xy[1]}};
        Vector2f src_position = src_center;
        src_position.z += m2f_transform(transform, from_center).z;
        if (src_position.xy[0] < 0.0f || src_position.xy[0] >= src.size.x ||
                src_position.xy[1] < 0.0f || src_position.xy[1] >= src.size.y) {
            PIXEL(dst, row, col) = bg_fill;
            continue;
        }
        PIXEL(dst, row, col) = interpolation(src, src_position);
    }
}

void img_histogram(uint32_t histogram[256], const ImageMatrix mat) {
    assert(histogram && IMG_IS_VALID(mat));
    for (uint16_t i = 0; i < 256; ++i) {
        histogram[i] = 0;
    }
    FOR_EACH_PIXEL(mat) { ++histogram[PIXEL(mat, row, col)]; }
}

uint8_t img_compute_otsu_threshold(const uint32_t histogram[256]) {
    assert(histogram);
    // "A C++ Implementation of Otsu’s Image Segmentation Method", 2016.
    int32_t N = 0;
    int32_t sum = 0;
    for (int16_t i = 0; i < 256; ++i) {
        N += histogram[i];
        sum += i * histogram[i];
    }
    uint8_t threshold = 0;
    int32_t sum_b = 0;
    int32_t q1 = 0;
    float max_variance = 0;
    for (int16_t i = 0; i < 256; ++i) {
        q1 += histogram[i];
        if (!q1) {
            continue;
        }
        int32_t q2 = N - q1;
        if (!q2) {
            break;
        }
        sum_b += i * histogram[i];
        float mean_difference = (float) sum_b / q1 - (float) (sum - sum_b) / q2;
        float variance = SQR(mean_difference) * q1 * q2;
        if (variance >= max_variance) {
            threshold = i;
            max_variance = variance;
        }
    }
    return threshold;
}

void img_draw_line(ImageMatrix mat, ImagePoint from, ImagePoint to, uint8_t color, uint8_t width) {
    // Bresenham's Line Algorithm
    assert(IMG_IS_VALID(mat) && width > 0);
    int16_t dx = to.x - from.x;
    int16_t dy = to.y - from.y;
    uint8_t swap_xy = ABS(dy) > ABS(dx);
    if (swap_xy) {
        SWAP(from.x, from.y);
        SWAP(to.x, to.y);
    }
    if (from.x > to.x) {
        SWAP(from.x, to.x);
        SWAP(from.y, to.y);
    }
    dx = to.x - from.x;
    dy = to.y - from.y;
    int8_t dir = 1;
    if (dy < 0) {
        dy = -dy;
        dir = -1;
    }
    float slope = (float) dy / dx;
    width *= sqrtf(1.0f + SQR(slope));
    int16_t error = dy * 2 - dx;
#define ITERATE_LINE(X, Y)                                                       \
    for (int16_t y = from.y, x = from.x; x <= to.x;                              \
            ++x, error += 2 * (dy - (error > 0) * dx), y += dir * (error > 0)) { \
        from.y = y - (width - 1) / 2;                                            \
        from.y = MAX(from.y, 0);                                                 \
        from.x = MAX(x, 0);                                                      \
        to.y = y + width / 2;                                                    \
        to.Y = MIN(to.Y, mat.size.y - 1);                                        \
        to.X = MIN(to.X, mat.size.x - 1);                                        \
        for (; from.y <= to.y; ++from.y) {                                       \
            PIXEL(mat, from.Y, from.X) = color;                                  \
        }                                                                        \
    }
    if (swap_xy) {
        ITERATE_LINE(y, x);
    } else {
        ITERATE_LINE(x, y);
    }
}

void img_draw_box(ImageMatrix mat, ImagePoint from, ImagePoint to, uint8_t color, uint8_t width) {
    assert(IMG_IS_VALID(mat) && width > 0);
    if (from.x > to.x) {
        SWAP(from.x, to.x);
    }
    if (from.y > to.y) {
        SWAP(from.y, to.y);
    }
    from.x = MAX(from.x, 0);
    from.y = MAX(from.y, 0);
    to.x = MIN(to.x, mat.size.x - 1);
    to.y = MIN(to.y, mat.size.y - 1);
    int16_t dx = to.x - from.x;
    int16_t dy = to.y - from.y;
    width = MIN(width, dx);
    width = MIN(width, dy);
    for (int16_t row = from.y; row <= to.y; ++row) {
        for (uint8_t i = 0; i < width; ++i) {
            PIXEL(mat, row, from.x + i) = color;
            PIXEL(mat, row, to.x - i) = color;
        }
    }
    for (int16_t col = from.x; col <= to.x; ++col) {
        for (uint8_t i = 0; i < width; ++i) {
            PIXEL(mat, from.y + i, col) = color;
            PIXEL(mat, to.y - i, col) = color;
        }
    }
}

void img_draw_polygon(
        ImageMatrix mat, const ImagePoint* vertices, uint8_t len, uint8_t color, uint8_t width) {
    assert(IMG_IS_VALID(mat) && vertices && len > 0 && width > 0);
    for (uint8_t prev = 0; len--; prev = len) {
        img_draw_line(mat, vertices[prev], vertices[len], color, width);
    }
}

void img_draw_regular_polygon(ImageMatrix mat, ImagePoint center, Vector2f center_to_vertex,
        uint8_t order, uint8_t color, uint8_t width) {
    assert(IMG_IS_VALID(mat));
    Vector2f rotation_increment = {{cosf(2 * M_PI_F / order), sinf(2 * M_PI_F / order)}};
    ImagePoint previous_vertex = {
            center.x + center_to_vertex.xy[0], center.y + center_to_vertex.xy[1]};
    for (uint8_t i = 0; i < order; ++i) {
        center_to_vertex.z *= rotation_increment.z;
        ImagePoint next_vertex = {
                center.x + center_to_vertex.xy[0], center.y + center_to_vertex.xy[1]};
        img_draw_line(mat, previous_vertex, next_vertex, color, width);
        previous_vertex = next_vertex;
    };
}

void img_hough_line_transform(ImageMatrixInt32 dst, const ImageMatrix src) {
    assert(IMG_IS_VALID(dst) && IMG_IS_VALID(src));
    IMG_FILL(dst, 0);
    float angle_resolution = M_PI_F / dst.size.y;
    float scale_to_index =
            dst.size.x / sqrtf((src.size.y * src.size.y) + (src.size.x * src.size.x));
    const Vector2f rot_inc = {{cosf(angle_resolution), sinf(angle_resolution)}};
    Vector2f rot = {{1, 0}};
    for (int16_t i = 0; i < dst.size.y; ++i) {
        rot.z *= rot_inc.z;
        FOR_EACH_PIXEL(src) {
            PIXEL(dst, i, (int16_t)(((rot.xy[1] * row) + (rot.xy[0] * col)) * scale_to_index)) +=
                    PIXEL(src, row, col);
        }
    }
    IMG_NORMALIZE(dst, dst);
}

void img_convert_from_rgb888(ImageMatrix dst, const uint8_t* src) {
    assert(src && IMG_IS_VALID(dst));
    int32_t data_len = IMG_PIXEL_COUNT(dst);
    for (int32_t i = 0; i < data_len; ++i, src += 3) {
        dst.data[i] = (src[0] + src[1] + src[2]) / 3;
    }
}

int img_save_to_pgm(ImageMatrix image, const char* file_name) {
    assert(file_name && IMG_IS_VALID(image));
    FILE* pgm_file = fopen(file_name, "wb");
    if (!pgm_file) {
        return -1;
    }
    if (0 > fprintf(pgm_file, "P5\n%u %u\n%u\n", image.size.x, image.size.y, 255)) {
        return ferror(pgm_file);
    }
    size_t len = IMG_PIXEL_COUNT(image);
    if (len != fwrite(image.data, 1, len, pgm_file)) {
        return ferror(pgm_file);
    }
    return fclose(pgm_file);
}
