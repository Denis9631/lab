#include "filters.h"
#include "headers.h"
void apply_crop(Image* image, int new_width, int new_height) {
    if (new_width <= 0 || new_height <= 0) return;
    int old_width = image->dib_header.width;
    int old_height = abs(image->dib_header.height);
    if (new_width > old_width) new_width = old_width;
    if (new_height > old_height) new_height = old_height;
    Pixel** new_pixels = (Pixel**)malloc(new_height * sizeof(Pixel*));
    for (int i = 0; i < new_height; i++) {
        new_pixels[i] = (Pixel*)malloc(new_width * sizeof(Pixel));
        for (int j = 0; j < new_width; j++) {
            new_pixels[i][j] = image->pixels[i][j];
        }
    }
    for (int i = 0; i < old_height; i++) {
        free(image->pixels[i]);
    }
    free(image->pixels);
    image->pixels = new_pixels;
    image->dib_header.width = new_width;
    image->dib_header.height = (image->dib_header.height > 0) ? new_height : -new_height;
}
void apply_grayscale(Image* image) {
    int width = image->dib_header.width;
    int height = abs(image->dib_header.height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Pixel* p = &image->pixels[y][x];
            uint8_t gray = (uint8_t)(0.299 * p->red + 0.587 * p->green + 0.114 * p->blue);
            p->red = p->green = p->blue = gray;
        }
    }
}
void apply_negative(Image* image) {
    int width = image->dib_header.width;
    int height = abs(image->dib_header.height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Pixel* p = &image->pixels[y][x];
            p->red = 255 - p->red;
            p->green = 255 - p->green;
            p->blue = 255 - p->blue;
        }
    }
}
void apply_sharpening(Image* image) {
    float kernel[3][3] = {
        {0, -1, 0},
        {-1, 5, -1},
        {0, -1, 0}
    };
    int width = image->dib_header.width;
    int height = abs(image->dib_header.height);
    Pixel** temp = (Pixel**)malloc(height * sizeof(Pixel*));
    for (int i = 0; i < height; i++) {
        temp[i] = (Pixel*)malloc(width * sizeof(Pixel));
        memcpy(temp[i], image->pixels[i], width * sizeof(Pixel));
    }
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sum_r = 0, sum_g = 0, sum_b = 0;
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int px = x + kx;
                    int py = y + ky;
                    if (px < 0) px = 0;
                    if (px >= width) px = width - 1;
                    if (py < 0) py = 0;
                    if (py >= height) py = height - 1;
                    
                    float weight = kernel[ky + 1][kx + 1];
                    sum_r += temp[py][px].red * weight;
                    sum_g += temp[py][px].green * weight;
                    sum_b += temp[py][px].blue * weight;
                }
            }
            image->pixels[y][x].red = (uint8_t)fmax(0, fmin(255, sum_r));
            image->pixels[y][x].green = (uint8_t)fmax(0, fmin(255, sum_g));
            image->pixels[y][x].blue = (uint8_t)fmax(0, fmin(255, sum_b));
        }
    }
    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
}
void apply_edge_threshold(Image* image, float threshold) {
    int width = image->dib_header.width;
    int height = abs(image->dib_header.height);
    apply_grayscale(image);
    Pixel** temp = (Pixel**)malloc(height * sizeof(Pixel*));
    for (int i = 0; i < height; i++) {
        temp[i] = (Pixel*)malloc(width * sizeof(Pixel));
        memcpy(temp[i], image->pixels[i], width * sizeof(Pixel));
    }
    float kernel[3][3] = {
        { 0, -1,  0},
        {-1,  4, -1},
        { 0, -1,  0}
    };
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sum = 0.0f;
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int px = x + kx;
                    int py = y + ky;
                    if (px < 0) px = 0;
                    if (px >= width) px = width - 1;
                    if (py < 0) py = 0;
                    if (py >= height) py = height - 1;
                    float pixel_value = temp[py][px].red / 255.0f;
                    float weight = kernel[ky + 1][kx + 1];
                    sum += pixel_value * weight;
                }
            }
            sum = fabs(sum);
            if (sum > threshold) {
                image->pixels[y][x].red = 255;
                image->pixels[y][x].green = 255;
                image->pixels[y][x].blue = 255;
            } else {
                image->pixels[y][x].red = 0;
                image->pixels[y][x].green = 0;
                image->pixels[y][x].blue = 0;
            }
        }
    }
    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
}
void apply_median_filter(Image* image, int window_size) {
    if (window_size < 3) window_size = 3;
    if (window_size > 15) window_size = 15;
    int width = image->dib_header.width;
    int height = abs(image->dib_header.height);
    int radius = window_size / 2;
    Pixel** temp = (Pixel**)malloc(height * sizeof(Pixel*));
    for (int i = 0; i < height; i++) {
        temp[i] = (Pixel*)malloc(width * sizeof(Pixel));
        memcpy(temp[i], image->pixels[i], width * sizeof(Pixel));
    }
    int max_pixels = window_size * window_size;
    uint8_t* red_values = (uint8_t*)malloc(max_pixels * sizeof(uint8_t));
    uint8_t* green_values = (uint8_t*)malloc(max_pixels * sizeof(uint8_t));
    uint8_t* blue_values = (uint8_t*)malloc(max_pixels * sizeof(uint8_t));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int count = 0;
            for (int wy = -radius; wy <= radius; wy++) {
                for (int wx = -radius; wx <= radius; wx++) {
                    int px = x + wx;
                    int py = y + wy;
                    if (px < 0) px = -px;
                    if (px >= width) px = 2 * width - px - 2;
                    if (py < 0) py = -py;
                    if (py >= height) py = 2 * height - py - 2;
                    red_values[count] = temp[py][px].red;
                    green_values[count] = temp[py][px].green;
                    blue_values[count] = temp[py][px].blue;
                    count++;
                }
            }
            for (int i = 0; i < count - 1; i++) {
                for (int j = 0; j < count - i - 1; j++) {
                    if (red_values[j] > red_values[j + 1]) {
                        uint8_t tmp = red_values[j];
                        red_values[j] = red_values[j + 1];
                        red_values[j + 1] = tmp;
                    }
                }
            }
            for (int i = 0; i < count - 1; i++) {
                for (int j = 0; j < count - i - 1; j++) {
                    if (green_values[j] > green_values[j + 1]) {
                        uint8_t tmp = green_values[j];
                        green_values[j] = green_values[j + 1];
                        green_values[j + 1] = tmp;
                    }
                }
            }
            for (int i = 0; i < count - 1; i++) {
                for (int j = 0; j < count - i - 1; j++) {
                    if (blue_values[j] > blue_values[j + 1]) {
                        uint8_t tmp = blue_values[j];
                        blue_values[j] = blue_values[j + 1];
                        blue_values[j + 1] = tmp;
                    }
                }
            }
            int median_index = count / 2;
            image->pixels[y][x].red = red_values[median_index];
            image->pixels[y][x].green = green_values[median_index];
            image->pixels[y][x].blue = blue_values[median_index];
        }
    }
    free(red_values);
    free(green_values);
    free(blue_values);
    for (int i = 0; i < height; i++) free(temp[i]);
    free(temp);
}
void apply_crystallize(Image* image, int radius) {
    int width = image->dib_header.width;
    int height = abs(image->dib_header.height);
    if (radius < 2) radius = 2;
    if (radius > 50) radius = 50;
    Pixel** temp = (Pixel**)malloc(height * sizeof(Pixel*));
    for (int i = 0; i < height; i++) {
        temp[i] = (Pixel*)malloc(width * sizeof(Pixel));
        memcpy(temp[i], image->pixels[i], width * sizeof(Pixel));
    }
    int cell_size = radius;
    for (int cell_y = 0; cell_y < height; cell_y += cell_size) {
        for (int cell_x = 0; cell_x < width; cell_x += cell_size) {
            int cell_end_x = cell_x + cell_size;
            int cell_end_y = cell_y + cell_size;
            if (cell_end_x > width) cell_end_x = width;
            if (cell_end_y > height) cell_end_y = height;
            int sum_r = 0, sum_g = 0, sum_b = 0;
            int count = 0;
            for (int y = cell_y; y < cell_end_y; y++) {
                for (int x = cell_x; x < cell_end_x; x++) {
                    sum_r += temp[y][x].red;
                    sum_g += temp[y][x].green;
                    sum_b += temp[y][x].blue;
                    count++;
                }
            }
            if (count > 0) {
                uint8_t avg_r = sum_r / count;
                uint8_t avg_g = sum_g / count;
                uint8_t avg_b = sum_b / count;
                for (int y = cell_y; y < cell_end_y; y++) {
                    for (int x = cell_x; x < cell_end_x; x++) {
                        image->pixels[y][x].red = avg_r;
                        image->pixels[y][x].green = avg_g;
                        image->pixels[y][x].blue = avg_b;
                    }
                }
            }
        }
    }
    for (int i = 0; i < height; i++) free(temp[i]);
    free(temp);
}
void apply_glass(Image* image, float scale) {
    int width = image->dib_header.width;
    int height = abs(image->dib_header.height);
    if (scale < 1.0f) scale = 1.0f;
    if (scale > 100.0f) scale = 100.0f;
    Pixel** temp = (Pixel**)malloc(height * sizeof(Pixel*));
    for (int i = 0; i < height; i++) {
        temp[i] = (Pixel*)malloc(width * sizeof(Pixel));
        memcpy(temp[i], image->pixels[i], width * sizeof(Pixel));
    }
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float offset_x = scale * sin((float)x / 20.0f) * cos((float)y / 25.0f);
            float offset_y = scale * cos((float)x / 22.0f) * sin((float)y / 18.0f);
            int new_x = x + (int)offset_x;
            int new_y = y + (int)offset_y;
            if (new_x < 0) new_x = 0;
            if (new_x >= width) new_x = width - 1;
            if (new_y < 0) new_y = 0;
            if (new_y >= height) new_y = height - 1;
            image->pixels[y][x] = temp[new_y][new_x];
        }
    }
    for (int i = 0; i < height; i++) free(temp[i]);
    free(temp);
}
void apply_blur(Image* image, float sigma) {
    if (sigma <= 0) sigma = 1.0f;
    int width = image->dib_header.width;
    int height = abs(image->dib_header.height);
    int radius = (int)(sigma * 2);
    if (radius < 1) radius = 1;
    if (radius > 10) radius = 10;
    Pixel** temp = (Pixel**)malloc(height * sizeof(Pixel*));
    for (int i = 0; i < height; i++) {
        temp[i] = (Pixel*)malloc(width * sizeof(Pixel));
        memcpy(temp[i], image->pixels[i], width * sizeof(Pixel));
    }
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int count = 0;
            int sum_r = 0, sum_g = 0, sum_b = 0;
            for (int ky = -radius; ky <= radius; ky++) {
                for (int kx = -radius; kx <= radius; kx++) {
                    int px = x + kx;
                    int py = y + ky;
                    if (px >= 0 && px < width && py >= 0 && py < height) {
                        sum_r += temp[py][px].red;
                        sum_g += temp[py][px].green;
                        sum_b += temp[py][px].blue;
                        count++;
                    }
                }
            }
            if (count > 0) {
                image->pixels[y][x].red = sum_r / count;
                image->pixels[y][x].green = sum_g / count;
                image->pixels[y][x].blue = sum_b / count;
            }
        }
    }
    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
}