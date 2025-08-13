#include <stdio.h>
#include <stdlib.h>

char brightness_to_ascii(unsigned char r, unsigned char g, unsigned char b) {
    // Convert RGB to grayscale
    unsigned char gray = (unsigned char)(0.299*r + 0.587*g + 0.114*b);

    // ASCII gradient from dark to light
    const char *ascii = "@%#*+=-:. ";
    int index = gray * 9 / 255;  // Map grayscale to gradient index
    return ascii[index];
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <rawfile> <width> <height>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int width = atoi(argv[2]);
    int height = atoi(argv[3]);

    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    size_t img_size = width * height * 3; // 3 bytes per pixel (RGB)
    unsigned char *data = malloc(img_size);
    if (!data) {
        perror("Memory allocation failed");
        fclose(file);
        return 1;
    }

    fread(data, 1, img_size, file);
    fclose(file);

    // Display image using ASCII brightness
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            unsigned char r = data[idx];
            unsigned char g = data[idx + 1];
            unsigned char b = data[idx + 2];
            putchar(brightness_to_ascii(r, g, b));
        }
        putchar('\n');
    }

    free(data);
    return 0;
}
