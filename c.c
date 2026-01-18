#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t type;              // 'BM'
    uint32_t size;              // Размер файла
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;            // Смещение до данных пикселей
} BMPHeader;

typedef struct {
    uint32_t size;              // Размер структуры (40)
    int32_t width;              // Ширина
    int32_t height;             // Высота (положительное - снизу вверх)
    uint16_t planes;            // Всегда 1
    uint16_t bits_per_pixel;    // 24
    uint32_t compression;       // 0 - без сжатия
    uint32_t image_size;        // 0 или размер данных
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;       // 0
    uint32_t colors_important;  // 0
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

Image* load_bmp(const char* filename);
void save_bmp(const char* filename, Image* image);
void free_image(Image* image);
void print_help();
void apply_crop(Image* image, int new_width, int new_height);
void apply_grayscale(Image* image);
void apply_negative(Image* image);
void apply_blur(Image* image, float sigma);
void apply_sharpening(Image* image);
void apply_edge_threshold(Image* image, float threshold);
void apply_median_filter(Image* image, int window_size);
void apply_crystallize(Image* image, int radius);
void apply_glass(Image* image, float scale);

int main(int argc, char* argv[]) {

    // Загружаем изображение
    Image* image = load_bmp(argv[1]);

    // Применяем фильтры
    for (int i = 3; i < argc; i++) {
        if (argv[i][0] == '-') {
            char* filter_name = argv[i] + 1; // Пропускаем '-'
            
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

    // Сохраняем результат
    save_bmp(argv[2], image);
    free_image(image);
    
    printf("Обработка завершена успешно!\n");
    return 0;
}

Image* load_bmp(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Не удалось открыть файл: %s\n", filename);
        return NULL;
    }

    Image* image = (Image*)malloc(sizeof(Image));
    
    // Читаем заголовки
    fread(&image->bmp_header, sizeof(BMPHeader), 1, file);
    fread(&image->dib_header, sizeof(DIBHeader), 1, file);
    
    // Проверяем формат
    if (image->dib_header.bits_per_pixel != 24) {
        printf("Только 24-битные BMP поддерживаются!\n");
        fclose(file);
        free(image);
        return NULL;
    }
    
    // Вычисляем ширину с учетом выравнивания
    int width = image->dib_header.width;
    int height = abs(image->dib_header.height);
    int padding = (4 - (width * 3) % 4) % 4;
    
    // Выделяем память для пикселей
    image->pixels = (Pixel**)malloc(height * sizeof(Pixel*));
    for (int i = 0; i < height; i++) {
        image->pixels[i] = (Pixel*)malloc(width * sizeof(Pixel));
    }
    
    // Переходим к данным пикселей
    fseek(file, image->bmp_header.offset, SEEK_SET);
    
    // Читаем пиксели
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            fread(&image->pixels[y][x], sizeof(Pixel), 1, file);
        }
        // Пропускаем выравнивание
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
    
    // Обновляем размер файла
    image->bmp_header.size = sizeof(BMPHeader) + sizeof(DIBHeader) + 
                             (width * 3 + padding) * height;
    
    // Записываем заголовки
    fwrite(&image->bmp_header, sizeof(BMPHeader), 1, file);
    fwrite(&image->dib_header, sizeof(DIBHeader), 1, file);
    
    // Пишем пиксели
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            fwrite(&image->pixels[y][x], sizeof(Pixel), 1, file);
        }
        // Добавляем выравнивание
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

void print_help() {
    printf("Использование: program.exe input.bmp output.bmp [фильтры]\n");
    printf("Доступные фильтры:\n");
    printf("  -crop width height    Обрезать изображение\n");
    printf("  -gs                   Оттенки серого\n");
    printf("  -neg                  Негатив\n");
    printf("  -sharp                Повышение резкости\n");
    printf("  -blur sigma           Размытие\n");
    printf("Пример: program.exe input.bmp output.bmp -crop 800 600 -gs\n");
}

void apply_crop(Image* image, int new_width, int new_height) {
    if (new_width <= 0 || new_height <= 0) return;
    
    int old_width = image->dib_header.width;
    int old_height = abs(image->dib_header.height);
    
    // Ограничиваем размеры
    if (new_width > old_width) new_width = old_width;
    if (new_height > old_height) new_height = old_height;
    
    // Создаем новый массив пикселей
    Pixel** new_pixels = (Pixel**)malloc(new_height * sizeof(Pixel*));
    for (int i = 0; i < new_height; i++) {
        new_pixels[i] = (Pixel*)malloc(new_width * sizeof(Pixel));
        // Копируем пиксели из верхнего левого угла
        for (int j = 0; j < new_width; j++) {
            new_pixels[i][j] = image->pixels[i][j];
        }
    }
    
    // Освобождаем старый массив
    for (int i = 0; i < old_height; i++) {
        free(image->pixels[i]);
    }
    free(image->pixels);
    
    // Обновляем структуру изображения
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
            // Формула для оттенков серого
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
    // Простая матрица повышения резкости
    float kernel[3][3] = {
        {0, -1, 0},
        {-1, 5, -1},
        {0, -1, 0}
    };
    
    int width = image->dib_header.width;
    int height = abs(image->dib_header.height);
    
    // Создаем копию для чтения
    Pixel** temp = (Pixel**)malloc(height * sizeof(Pixel*));
    for (int i = 0; i < height; i++) {
        temp[i] = (Pixel*)malloc(width * sizeof(Pixel));
        memcpy(temp[i], image->pixels[i], width * sizeof(Pixel));
    }
    
    // Применяем фильтр
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sum_r = 0, sum_g = 0, sum_b = 0;
            
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int px = x + kx;
                    int py = y + ky;
                    
                    // Обработка границ
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
            
            // Ограничиваем значения
            image->pixels[y][x].red = (uint8_t)fmax(0, fmin(255, sum_r));
            image->pixels[y][x].green = (uint8_t)fmax(0, fmin(255, sum_g));
            image->pixels[y][x].blue = (uint8_t)fmax(0, fmin(255, sum_b));
        }
    }
    
    // Освобождаем временную память
    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
}

void apply_edge_threshold(Image* image, float threshold) {
    int width = image->dib_header.width;
    int height = abs(image->dib_header.height);
    
    // 1. Сначала переводим в оттенки серого
    apply_grayscale(image);
    
    // 2. Создаем временную копию
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
    
    // 4. Применяем фильтр
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sum = 0.0f;
            
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int px = x + kx;
                    int py = y + ky;
                    
                    // Обработка границ: используем ближайший пиксель
                    if (px < 0) px = 0;
                    if (px >= width) px = width - 1;
                    if (py < 0) py = 0;
                    if (py >= height) py = height - 1;
                    
                    float pixel_value = temp[py][px].red / 255.0f;
                    float weight = kernel[ky + 1][kx + 1];
                    sum += pixel_value * weight;
                }
            }
            
            // Абсолютное значение градиента
            sum = fabs(sum);
            
            // 5. Применяем порог
            if (sum > threshold) {
                // Белый пиксель
                image->pixels[y][x].red = 255;
                image->pixels[y][x].green = 255;
                image->pixels[y][x].blue = 255;
            } else {
                // Черный пиксель
                image->pixels[y][x].red = 0;
                image->pixels[y][x].green = 0;
                image->pixels[y][x].blue = 0;
            }
        }
    }
    
    // 6. Освобождаем память
    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
}

void apply_median_filter(Image* image, int window_size) {
    // Проверяем, что окно нечетное
    if (window_size % 2 == 0) {
        printf("Ошибка: размер окна должен быть нечетным\n");
        return;
    }
    
    if (window_size < 3) window_size = 3;
    if (window_size > 15) window_size = 15; // Ограничим для скорости
    
    int width = image->dib_header.width;
    int height = abs(image->dib_header.height);
    int radius = window_size / 2;
    
    // Создаем временную копию
    Pixel** temp = (Pixel**)malloc(height * sizeof(Pixel*));
    for (int i = 0; i < height; i++) {
        temp[i] = (Pixel*)malloc(width * sizeof(Pixel));
        memcpy(temp[i], image->pixels[i], width * sizeof(Pixel));
    }
    
    // Буферы для хранения значений из окна
    int max_pixels = window_size * window_size;
    uint8_t* red_values = (uint8_t*)malloc(max_pixels * sizeof(uint8_t));
    uint8_t* green_values = (uint8_t*)malloc(max_pixels * sizeof(uint8_t));
    uint8_t* blue_values = (uint8_t*)malloc(max_pixels * sizeof(uint8_t));
    
    // Применяем медианный фильтр
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int count = 0;
            
            // Собираем все пиксели в окрестности
            for (int wy = -radius; wy <= radius; wy++) {
                for (int wx = -radius; wx <= radius; wx++) {
                    int px = x + wx;
                    int py = y + wy;
                    
                    // Обработка границ: отражаем
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
            
            // Сортируем каждый канал отдельно (пузырьковая сортировка - простая)
            // Красный канал
            for (int i = 0; i < count - 1; i++) {
                for (int j = 0; j < count - i - 1; j++) {
                    if (red_values[j] > red_values[j + 1]) {
                        uint8_t tmp = red_values[j];
                        red_values[j] = red_values[j + 1];
                        red_values[j + 1] = tmp;
                    }
                }
            }
            
            // Зеленый канал
            for (int i = 0; i < count - 1; i++) {
                for (int j = 0; j < count - i - 1; j++) {
                    if (green_values[j] > green_values[j + 1]) {
                        uint8_t tmp = green_values[j];
                        green_values[j] = green_values[j + 1];
                        green_values[j + 1] = tmp;
                    }
                }
            }
            
            // Синий канал
            for (int i = 0; i < count - 1; i++) {
                for (int j = 0; j < count - i - 1; j++) {
                    if (blue_values[j] > blue_values[j + 1]) {
                        uint8_t tmp = blue_values[j];
                        blue_values[j] = blue_values[j + 1];
                        blue_values[j + 1] = tmp;
                    }
                }
            }
            
            // Берем медианное значение (середина отсортированного массива)
            int median_index = count / 2;
            image->pixels[y][x].red = red_values[median_index];
            image->pixels[y][x].green = green_values[median_index];
            image->pixels[y][x].blue = blue_values[median_index];
        }
    }
    
    // Освобождаем память
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
    
    // Создаем временную копию
    Pixel** temp = (Pixel**)malloc(height * sizeof(Pixel*));
    for (int i = 0; i < height; i++) {
        temp[i] = (Pixel*)malloc(width * sizeof(Pixel));
        memcpy(temp[i], image->pixels[i], width * sizeof(Pixel));
    }
    
    // Размер ячейки
    int cell_size = radius;
    
    // Делим изображение на квадратные ячейки
    for (int cell_y = 0; cell_y < height; cell_y += cell_size) {
        for (int cell_x = 0; cell_x < width; cell_x += cell_size) {
            // Находим границы ячейки
            int cell_end_x = cell_x + cell_size;
            int cell_end_y = cell_y + cell_size;
            if (cell_end_x > width) cell_end_x = width;
            if (cell_end_y > height) cell_end_y = height;
            
            // Считаем средний цвет в ячейке
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
                
                // Закрашиваем ячейку средним цветом
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
    
    // Ограничиваем scale
    if (scale < 1.0f) scale = 1.0f;
    if (scale > 100.0f) scale = 100.0f;
    // Создаем временную копию
    Pixel** temp = (Pixel**)malloc(height * sizeof(Pixel*));
    for (int i = 0; i < height; i++) {
        temp[i] = (Pixel*)malloc(width * sizeof(Pixel));
        memcpy(temp[i], image->pixels[i], width * sizeof(Pixel));
    }
    
    // Создаем простую "текстуру" (синусоидальный паттерн)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Вычисляем смещение на основе текстуры
            float offset_x = scale * sin((float)x / 20.0f) * cos((float)y / 25.0f);
            float offset_y = scale * cos((float)x / 22.0f) * sin((float)y / 18.0f);
            
            // Новые координаты с учетом смещения
            int new_x = x + (int)offset_x;
            int new_y = y + (int)offset_y;
            
            // Ограничиваем границы
            if (new_x < 0) new_x = 0;
            if (new_x >= width) new_x = width - 1;
            if (new_y < 0) new_y = 0;
            if (new_y >= height) new_y = height - 1;
            
            // Берем цвет из смещенной позиции
            image->pixels[y][x] = temp[new_y][new_x];
        }
    }
    
    // Освобождаем память
    for (int i = 0; i < height; i++) free(temp[i]);
    free(temp);
}

void apply_blur(Image* image, float sigma) {
    if (sigma <= 0) sigma = 1.0f;
    
    int width = image->dib_header.width;
    int height = abs(image->dib_header.height);
    
    // Простое box blur с радиусом в зависимости от sigma
    int radius = (int)(sigma * 2);
    if (radius < 1) radius = 1;
    if (radius > 10) radius = 10;
    
    // Создаем временную копию
    Pixel** temp = (Pixel**)malloc(height * sizeof(Pixel*));
    for (int i = 0; i < height; i++) {
        temp[i] = (Pixel*)malloc(width * sizeof(Pixel));
        memcpy(temp[i], image->pixels[i], width * sizeof(Pixel));
    }
    
    // Box blur
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
    
    // Освобождаем память
    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
}