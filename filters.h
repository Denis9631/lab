#ifndef FILTERS_H
#define FILTERS_H
#include "headers.h"
void apply_crop(Image* image, int new_width, int new_height);
void apply_grayscale(Image* image);
void apply_negative(Image* image);
void apply_blur(Image* image, float sigma);
void apply_sharpening(Image* image);
void apply_edge_threshold(Image* image, float threshold);
void apply_median_filter(Image* image, int window_size);
void apply_crystallize(Image* image, int radius);
void apply_glass(Image* image, float scale);
#endif