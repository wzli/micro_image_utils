# Micro Image Utils

A light weight and portable image processing library designed for resource constrained embedded micro-controllers.
### Features
- Zero dynamic memory allocation (manual frame buffer management).
- All algorithms implemented without use of intermediate working buffers. 
- C99 compliant, no external dependencies required.


## Functions

Summary of provided functions (see [image_utils.h](./include/image_utils.h) for full list and details) :

### Image Editing
- Crop, Resize, Rotate, Skew, Flip, Normalize
- Nearest, Bi-linear, Bi-cubic Interpolation
- Draw Lines, Rectangles, Polygons

### Filters
- Convolution Filters (Blur, Sharpen, Edge Detect, Sobel, etc)
- Max/Min Filters (Dilate/Erode)
- Median Filter (Noise Rejection)
- Binary Thresholding (Otsu's Method)

### Domain Transforms
- Fast Fourier Transform
- Euclidian Distance Transform
- Hough Line Transform

## Usage
```bash
# clone the repo
git clone --recurse-submodules https://github.com/wzli/micro_image_utils.git

# run unit tests
cd micro_image_utils
make
./build/test
```
Refer to the [Makefile](./Makefile) and [Unit Tests](./src/tests/test_image_utils.c) for integration and usage example.

## Limitations
- Single-threaded
- Does not use processor specific optimizations (except compiler provided ones)
- Does not contain image (de)compression utilities (Eg JPEG, PNG, etc)
- Most functions assume single channel data (requires manual channel separation)
