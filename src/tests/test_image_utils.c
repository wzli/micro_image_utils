#include "test_utils.h"
#include "image_utils.h"

static ImageMatrix test_img;
static ImageMatrix buf_img;

static int test_image_size() {
    test_assert(IMG_PIXEL_COUNT(test_img) == 25);
    return 0;
}

static int test_image_set_pixels() {
    for (int i = 0; i < IMG_PIXEL_COUNT(test_img); ++i) {
        test_img.data[i] = i;
    }
    int i = 0;
    FOR_EACH_PIXEL(test_img) { test_assert(PIXEL(test_img, row, col) == i++); };
    return 0;
}

static int test_image_copy() {
    IMG_COPY(buf_img, test_img);
    FOR_EACH_PIXEL(test_img) {
        test_assert(PIXEL(test_img, row, col) == PIXEL(buf_img, row, col));
    };
    return 0;
}

static int test_image_fill() {
    IMG_FILL(buf_img, 0);
    FOR_EACH_PIXEL(test_img) { test_assert(PIXEL(buf_img, row, col) == 0); };
    return 0;
}

static int test_image_pixel_sum() {
    int sum = 0;
    IMG_PIXEL_SUM(sum, test_img);
    test_assert(sum == 300);
    return 0;
}

static int test_image_pixel_average() {
    int avg = 0;
    IMG_PIXEL_AVERAGE(avg, test_img);
    test_assert(avg == 12);
    return 0;
}

static int test_image_pixel_max() {
    uint8_t max = 0;
    IMG_PIXEL_MAX(max, test_img);
    test_assert(max == 24);
    return 0;
}

static int test_image_pixel_min() {
    uint8_t min = UINT8_MAX;
    IMG_PIXEL_MIN(min, test_img);
    test_assert(min == 0);
    return 0;
}

static inline bool latch_comp(uint8_t cur_val, uint8_t min_val) {
    return cur_val < min_val;
}

static int test_image_pixel_latch_index() {
    uint8_t min = UINT8_MAX;
    uint8_t min_x = -1;
    uint8_t min_y = -1;
    IMG_PIXEL_LATCH_INDEX(min, min_y, min_x, latch_comp, test_img);
    test_assert(min == 0);
    test_assert(min_x == 0);
    test_assert(min_y == 0);
    return 0;
}

static int test_image_apply_kernel() {
    int32_t acc = 0;
    IMG_PIXEL_WEIGHTED_SUM(acc, img_edge_detect_kernel, test_img, 0, 0);
    test_assert(acc == 0);
    return 0;
}

static int test_image_threshold() {
    IMG_THRESHOLD(buf_img, test_img, 4);
    FOR_EACH_PIXEL(buf_img) { test_assert(PIXEL(buf_img, row, col) == (row > 0) * UINT8_MAX); }
    return 0;
}

static int test_image_crop() {
    IMG_SET_SIZE(buf_img, 2, 2);
    ImagePoint top_left = {3, 3};
    IMG_CROP(buf_img, test_img, top_left);
    test_assert(IMG_PIXEL_COUNT(buf_img) == 4);
    FOR_EACH_PIXEL(buf_img) {
        test_assert(PIXEL(buf_img, row, col) == PIXEL(test_img, row + 3, col + 3));
    }
    return 0;
}

static int test_image_paste() {
    IMG_SET_SIZE(buf_img, 7, 7);
    IMG_FILL(buf_img, 0);
    ImagePoint top_left = {-1, -1};
    IMG_PASTE(buf_img, test_img, top_left);
    FOR_EACH_PIXEL(buf_img) { test_assert(PIXEL(buf_img, row, col) == 0); }
    top_left.x = 0;
    top_left.y = 0;
    IMG_PASTE(buf_img, test_img, top_left);
    FOR_EACH_PIXEL(buf_img) {
        test_assert(PIXEL(buf_img, row, col) == ((row >= test_img.size.y || col >= test_img.size.x)
                                                                ? 0
                                                                : PIXEL(test_img, row, col)));
    }
    return 0;
}

static int test_image_transpose() {
    IMG_TRANSPOSE(buf_img, test_img);
    FOR_EACH_PIXEL(buf_img) { test_assert(PIXEL(buf_img, row, col) == PIXEL(test_img, col, row)); }

    IMG_TRANSPOSE(buf_img, buf_img);
    FOR_EACH_PIXEL(test_img) {
        test_assert(PIXEL(test_img, row, col) == PIXEL(buf_img, row, col));
    };

    buf_img.size.x = 4;
    IMG_TRANSPOSE(buf_img, buf_img);
    test_assert(IMG_PIXEL_COUNT(buf_img) == 0);
    return 0;
}

static int test_image_vflip() {
    IMG_VFLIP(buf_img, test_img);
    FOR_EACH_PIXEL(buf_img) {
        test_assert(PIXEL(buf_img, row, col) == PIXEL(test_img, test_img.size.y - 1 - row, col));
    }

    IMG_VFLIP(buf_img, buf_img);
    FOR_EACH_PIXEL(test_img) {
        test_assert(PIXEL(test_img, row, col) == PIXEL(buf_img, row, col));
    };
    return 0;
}

static int test_image_hflip() {
    IMG_HFLIP(buf_img, test_img);
    FOR_EACH_PIXEL(buf_img) {
        test_assert(PIXEL(buf_img, row, col) == PIXEL(test_img, row, test_img.size.x - 1 - col));
    }

    IMG_HFLIP(buf_img, buf_img);
    FOR_EACH_PIXEL(test_img) {
        test_assert(PIXEL(test_img, row, col) == PIXEL(buf_img, row, col));
    };
    return 0;
}

static int test_image_normalize() {
    IMG_NORMALIZE_RANGE(buf_img, test_img, 5, 20);
    test_assert(buf_img.data[5] == 0);
    test_assert(buf_img.data[20] == UINT8_MAX);

    IMG_NORMALIZE(buf_img, test_img);
    test_assert(PIXEL(buf_img, 0, 0) == 0);
    test_assert(PIXEL(buf_img, 4, 4) == UINT8_MAX);
    return 0;
}

static int test_image_otsu_histogram() {
    uint32_t histogram[256];
    img_histogram(histogram, test_img);
    for (int i = 0; i < 256; ++i) {
        test_assert(histogram[i] == (i < 25));
    }
    test_assert(img_compute_otsu_threshold(histogram) == 12);
    return 0;
}

static int test_image_resize() {
    buf_img.size.x *= 2;
    img_resize(buf_img, test_img, img_nearest_interpolation);
    FOR_EACH_PIXEL(buf_img) {
        test_assert(PIXEL(buf_img, row, col) == PIXEL(test_img, row, col / 2));
    }
    return 0;
}

static int test_image_draw_line() {
    IMG_COPY(buf_img, test_img);
    img_draw_line(buf_img, (ImagePoint){0, 0}, (ImagePoint){5, 0}, 255, 2);
    FOR_EACH_PIXEL(buf_img) { test_assert((row < 2) == (PIXEL(buf_img, row, col) == 255)); }
    return 0;
}

static int test_image_draw_box() {
    IMG_FILL(buf_img, 0);
    img_draw_box(buf_img, (ImagePoint){-3, -7}, (ImagePoint){10, 20}, 255, 100);
    FOR_EACH_PIXEL(buf_img) { test_assert(PIXEL(buf_img, row, col) == 255); }

    IMG_FILL(buf_img, 0);
    img_draw_box(buf_img, (ImagePoint){1, 1}, (ImagePoint){3, 3}, 255, 1);
    int sum = 0;
    IMG_PIXEL_SUM(sum, buf_img);
    test_assert(sum == 8 * 255);
    img_draw_box(buf_img, (ImagePoint){1, 1}, (ImagePoint){3, 3}, 255, 2);
    sum = 0;
    IMG_PIXEL_SUM(sum, buf_img);
    test_assert(sum == 9 * 255);
    return 0;
}

static int test_image_manhattan_distance_transform() {
    ImageMatrixInt32 dist_img = {(int32_t[5 * 5]){0}, {5, 5}};
    IMG_FILL(dist_img, 255);
    img_manhattan_distance_transform(dist_img);
    FOR_EACH_PIXEL(dist_img) { test_assert(PIXEL(dist_img, row, col) == 255); }

    PIXEL(dist_img, 0, 0) = 0;
    img_manhattan_distance_transform(dist_img);
    FOR_EACH_PIXEL(dist_img) { test_assert(PIXEL(dist_img, row, col) == row + col); }
    return 0;
}

static int test_image_square_distance_transform() {
    ImageMatrixInt32 dist_img = {(int32_t[6 * 6]){0}, {5, 5}};
    IMG_FILL(dist_img, 255);
    img_square_distance_transform(dist_img);
    FOR_EACH_PIXEL(dist_img) { test_assert(PIXEL(dist_img, row, col) == 255); }

    PIXEL(dist_img, 0, 0) = 0;
    img_square_distance_transform(dist_img);
    FOR_EACH_PIXEL(dist_img) { test_assert(PIXEL(dist_img, row, col) == SQR(row) + SQR(col)); }
    return 0;
}

static int test_image_max_filter() {
    ImageMatrix src_img = {(uint8_t[5 * 5]){0}, {5, 5}};
    IMG_FILL(src_img, 0);
    PIXEL(src_img, 2, 2) = 255;
    img_max_filter(&buf_img, src_img, 2);
    FOR_EACH_PIXEL(buf_img) {
        if (row == 0 || row == buf_img.size.y - 1 || col == 0 || col == buf_img.size.x - 1) {
            test_assert(PIXEL(buf_img, row, col) == 0);
        } else {
            test_assert(PIXEL(buf_img, row, col) == 255);
        }
    }
    return 0;
}

static int test_image_min_filter() {
    ImageMatrix src_img = {(uint8_t[5 * 5]){0}, {5, 5}};
    IMG_FILL(src_img, 255);
    PIXEL(src_img, 2, 2) = 0;
    img_min_filter(&buf_img, src_img, 2);
    FOR_EACH_PIXEL(buf_img) {
        if (row == 0 || row == buf_img.size.y - 1 || col == 0 || col == buf_img.size.x - 1) {
            test_assert(PIXEL(buf_img, row, col) == 255);
        } else {
            test_assert(PIXEL(buf_img, row, col) == 0);
        }
    }
    return 0;
}

static int test_image_fast_fourier_transform() {
    ImageMatrixComplex img = {(Vector2f[4 * 4]){{{0}}}, {4, 4}};
    Vector2f fill_val = {{1, 0}};
    IMG_FILL(img, fill_val);
    img_fast_fourier_transform(img, false);
    FOR_EACH_PIXEL(img) { test_assert(PIXEL(img, row, col).z == (!row && !col) * 16); }
    img_fast_fourier_transform(img, true);
    FOR_EACH_PIXEL(img) { test_assert(PIXEL(img, row, col).z == 1); }
    return 0;
}

int test_image_utils() {
    uint8_t test_img_data[5 * 5];
    uint8_t buf_img_data[5 * 5 * 2];
    test_img = (ImageMatrix){test_img_data, {5, 5}};
    buf_img = (ImageMatrix){buf_img_data, {5, 5}};
    test_run(test_image_size);
    test_run(test_image_set_pixels);
    test_run(test_image_copy);
    test_run(test_image_fill);
    test_run(test_image_pixel_sum);
    test_run(test_image_pixel_average);
    test_run(test_image_pixel_max);
    test_run(test_image_pixel_min);
    test_run(test_image_pixel_latch_index);
    test_run(test_image_apply_kernel);
    test_run(test_image_threshold);
    test_run(test_image_crop);
    test_run(test_image_paste);
    test_run(test_image_transpose);
    test_run(test_image_vflip);
    test_run(test_image_hflip);
    test_run(test_image_normalize);
    test_run(test_image_otsu_histogram);
    test_run(test_image_resize);
    test_run(test_image_draw_line);
    test_run(test_image_draw_box);
    test_run(test_image_manhattan_distance_transform);
    test_run(test_image_square_distance_transform);
    test_run(test_image_max_filter);
    test_run(test_image_min_filter);
    test_run(test_image_fast_fourier_transform);
    return 0;
}
