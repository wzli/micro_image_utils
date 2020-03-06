#pragma once
#include "generic_image_utils.h"
#include "math_utils.h"
#include "mfft.h"
#include "distance_transform.h"

IMG_MATRIX_TYPEDEF(ImageMatrix, uint8_t);
IMG_MATRIX_TYPEDEF(ImageMatrixInt8, int8_t);
IMG_MATRIX_TYPEDEF(ImageMatrixInt32, int32_t);
IMG_MATRIX_TYPEDEF(ImageMatrixComplex, Vector2f);

// basic type generic functions

// PIXEL(MAT, ROW, COL)
// IMG_COPY(DST, SRC)
// IMG_FILL(MAT, VAL)
// IMG_PIXEL_SUM(SUM, MAT)
// IMG_PIXEL_AVERAGE(AVG, MAT)
// IMG_PIXEL_MAX(MAX_VAL, MAT)
// IMG_PIXEL_MIN(MIN_VAL, MAT)
// IMG_THRESHOLD(DST, SRC, THRESH)
// IMG_NORMALIZE(DST, SRC)
// IMG_NORMALIZE_RANGE(DST, SRC, MIN, MAX)
// IMG_CROP(DST, SRC, TOP_LEFT)
// IMG_VFLIP(DST, SRC)
// IMG_HFLIP(DST, SRC)

// convolution kernels
static const ImageMatrixInt8 img_sharpen_kernel = {
        (int8_t[]){-1, -1, -1, -1, 9, -1, -1, -1, -1}, {3, 3}};
static const ImageMatrixInt8 img_edge_detect_kernel = {
        (int8_t[]){-1, -1, -1, -1, 8, -1, -1, -1, -1}, {3, 3}};
static const ImageMatrixInt8 img_sobel_x_kernel = {
        (int8_t[]){-1, 0, 1, -2, 0, 2, -1, 0, 1}, {3, 3}};
static const ImageMatrixInt8 img_sobel_y_kernel = {
        (int8_t[]){-1, -2, -1, 0, 0, 0, 1, 2, 1}, {3, 3}};
static const ImageMatrixInt8 img_laplacian_kernel = {
        (int8_t[]){0, 1, 0, 1, -4, 1, 0, 1, 0}, {3, 3}};

// filters (they also work inplace)
void img_convolution_filter(ImageMatrix* dst, const ImageMatrix src, const ImageMatrixInt8 kernel);
void img_max_filter(ImageMatrix* dst, const ImageMatrix src, uint16_t block_size);
void img_min_filter(ImageMatrix* dst, const ImageMatrix src, uint16_t block_size);
void img_median_filter(ImageMatrix* dst, const ImageMatrix src, ImageMatrix window);

// interpolation methods
typedef uint8_t (*ImageInterpolation)(const ImageMatrix mat, Vector2f position);
uint8_t img_nearest_interpolation(const ImageMatrix mat, Vector2f position);
uint8_t img_bilinear_interpolation(const ImageMatrix mat, Vector2f position);
uint8_t img_bicubic_interpolation(const ImageMatrix mat, Vector2f position);

// geometric transformations
void img_resize(ImageMatrix dst, const ImageMatrix src, ImageInterpolation interpolation);
void img_rotate(ImageMatrix dst, const ImageMatrix src, Vector2f rotation, uint8_t bg_fill,
        ImageInterpolation interpolation);
void img_affine_transform(ImageMatrix dst, const ImageMatrix src, Matrix2f transform,
        uint8_t bg_fill, ImageInterpolation interpolation);

// histogram thresholding
void img_histogram(uint32_t histogram[256], const ImageMatrix mat);
uint8_t img_compute_otsu_threshold(const uint32_t histogram[256]);

// drawing utilitites
void img_draw_line(ImageMatrix mat, ImagePoint from, ImagePoint to, uint8_t color, uint8_t width);
void img_draw_box(ImageMatrix mat, ImagePoint from, ImagePoint to, uint8_t color, uint8_t width);
void img_draw_polygon(
        ImageMatrix mat, const ImagePoint* vertices, uint8_t len, uint8_t color, uint8_t width);
void img_draw_regular_polygon(ImageMatrix mat, ImagePoint center, Vector2f center_to_vertex,
        uint8_t order, uint8_t color, uint8_t width);

// domain transforms
void img_hough_line_transform(ImageMatrixInt32 dst, const ImageMatrix src);

static inline void img_fast_fourier_transform(ImageMatrixComplex mat, bool inverse) {
    fft_2d(&mat.data->z, mat.size.x, mat.size.y, inverse);
}
static inline void img_manhattan_distance_transform(ImageMatrixInt32 mat) {
    manhattan_distance_transform_2d(mat.data, mat.data, mat.size.x, mat.size.y);
}
// square distance transform REQUIRES buffer to be size (x_len + 1)*(y_len + 1)
static inline void img_square_distance_transform(ImageMatrixInt32 mat) {
    square_distance_transform_2d(mat.data, mat.data, mat.size.x, mat.size.y);
}

// format conversions
void img_convert_from_rgb888(ImageMatrix* dst, const ImageMatrix src);
int img_save_to_pgm(ImageMatrix image, const char* file_name);
