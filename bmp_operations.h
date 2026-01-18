#ifndef BMP_OPERATIONS_H
#define BMP_OPERATIONS_H
#include "headers.h"
Image* load_bmp(const char* filename);
void save_bmp(const char* filename, Image* image);
void free_image(Image* image);
#endif