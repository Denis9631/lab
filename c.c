#include "headers.h"
#include "bmp_operations.h"
#include "filters.h"
int main(int argc, char* argv[]) {
    Image* image = load_bmp(argv[1]);
    for (int i = 3; i < argc; i++) {
        if (argv[i][0] == '-') {
            char* filter_name = argv[i] + 1; 
            if (strcmp(filter_name, "crop") == 0 && i + 2 < argc) {
                int width = atoi(argv[i + 1]);
                int height = atoi(argv[i + 2]);
                apply_crop(image, width, height);
                i += 2;
            }
            else if (strcmp(filter_name, "gs") == 0) {
                apply_grayscale(image);
            }
            else if (strcmp(filter_name, "neg") == 0) {
                apply_negative(image);
            }
            else if (strcmp(filter_name, "sharp") == 0) {
                apply_sharpening(image);
            }
            else if (strcmp(filter_name, "blur") == 0 && i + 1 < argc) {
                float sigma = atof(argv[i + 1]);
                apply_blur(image, sigma);
                i += 1;
            }
            else if (strcmp(filter_name, "edge") == 0 && i + 1 < argc) {
                float threshold = atof(argv[i + 1]);
                apply_edge_threshold(image, threshold);
                i += 1;
            }
            else if (strcmp(filter_name, "med") == 0 && i + 1 < argc) {
                int window_size = atoi(argv[i + 1]);
                apply_median_filter(image, window_size);
                i += 1;
            }
            else if (strcmp(filter_name, "crystal") == 0 && i + 1 < argc) {
                int radius = atoi(argv[i + 1]);
                apply_crystallize(image, radius);
                i += 1;
            }
            else if (strcmp(filter_name, "glass") == 0 && i + 1 < argc) {
                float scale = atof(argv[i + 1]);
                apply_glass(image, scale);
                i += 1;
            }
            else {
                printf("Неизвестный фильтр: %s\n", filter_name);
            }
        }
    }
    save_bmp(argv[2], image);
    free_image(image);
    return 0;
}