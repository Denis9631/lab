#include "bmp_operations.h"
Image* load_bmp(const char* filename) {
    FILE* file = fopen(filename, "rb");
    Image* image = (Image*)malloc(sizeof(Image));
    fread(&image->bmp_header, sizeof(BMPHeader), 1, file);
    fread(&image->dib_header, sizeof(DIBHeader), 1, file);
    int width = image->dib_header.width;
    int height = abs(image->dib_header.height);
    int padding = (4 - (width * 3) % 4) % 4;
    image->pixels = (Pixel**)malloc(height * sizeof(Pixel*));
    for (int i = 0; i < height; i++) {
        image->pixels[i] = (Pixel*)malloc(width * sizeof(Pixel));
    }
    fseek(file, image->bmp_header.offset, SEEK_SET);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            fread(&image->pixels[y][x], sizeof(Pixel), 1, file);
        }
        fseek(file, padding, SEEK_CUR);
    }
    fclose(file);
    return image;
}
void save_bmp(const char* filename, Image* image) {
    FILE* file = fopen(filename, "wb");
    if (!file) return;
    int width = image->dib_header.width;
    int height = abs(image->dib_header.height);
    int padding = (4 - (width * 3) % 4) % 4;
    image->bmp_header.size = sizeof(BMPHeader) + sizeof(DIBHeader) + 
                             (width * 3 + padding) * height;
    fwrite(&image->bmp_header, sizeof(BMPHeader), 1, file);
    fwrite(&image->dib_header, sizeof(DIBHeader), 1, file);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            fwrite(&image->pixels[y][x], sizeof(Pixel), 1, file);
        }
        uint8_t pad_byte = 0;
        for (int i = 0; i < padding; i++) {
            fwrite(&pad_byte, 1, 1, file);
        }
    }
    fclose(file);
}
void free_image(Image* image) {
    if (!image) return;
    int height = abs(image->dib_header.height);
    for (int i = 0; i < height; i++) {
        free(image->pixels[i]);
    }
    free(image->pixels);
    free(image);
}