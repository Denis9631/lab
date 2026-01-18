#ifndef HEADERS_H
#define HEADERS_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPHeader;
typedef struct {
    uint32_t size;      
    int32_t width;      
    int32_t height;          
    uint16_t planes;  
    uint16_t bits_per_pixel;   
    uint32_t compression;   
    uint32_t image_size;     
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;   
    uint32_t colors_important;  
} DIBHeader;
#pragma pack(pop)
typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} Pixel;
typedef struct {
    BMPHeader bmp_header;
    DIBHeader dib_header;
    Pixel** pixels;
} Image;
#endif