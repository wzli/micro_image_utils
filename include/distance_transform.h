#pragma once

// 1D + 2D manhattan distance transforms work in-place by setting dst = src
void manhattan_distance_transform_1d(int* dst, const int* src, int len, int stride);
void manhattan_distance_transform_2d(int* dst, const int* src, int x_len, int y_len);

// 1D square distance transform DOES NOT work in-place
void square_distance_transform_1d(int* dst, const int* src, int len, int stride);
// 2D version works in-place, also REQUIRES dst buffer to be size (x_len + 1)*(y_len + 1)
void square_distance_transform_2d(int* dst, const int* src, int x_len, int y_len);
