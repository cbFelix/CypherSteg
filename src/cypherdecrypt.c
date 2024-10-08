#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

void create_output_directories() {
    struct stat st = {0};

    // Create output/decrypt if it does not exist
    if (stat("output/decrypt", &st) == -1) {
        mkdir("output", 0700);
        mkdir("output/decrypt", 0700);
    }
}

void read_png_file(const char *filename, png_bytep **row_pointers, png_structp *png_ptr, png_infop *info_ptr, int *width, int *height, int *color_type, int *bit_depth) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Error opening PNG file");
        exit(EXIT_FAILURE);
    }

    unsigned char header[8];
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)) {
        fprintf(stderr, "File is not a PNG.\n");
        exit(EXIT_FAILURE);
    }

    *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!*png_ptr) {
        fprintf(stderr, "Failed to create png_struct.\n");
        exit(EXIT_FAILURE);
    }

    *info_ptr = png_create_info_struct(*png_ptr);
    if (!*info_ptr) {
        fprintf(stderr, "Failed to create png_info.\n");
        exit(EXIT_FAILURE);
    }

    if (setjmp(png_jmpbuf(*png_ptr))) {
        fprintf(stderr, "Error during PNG initialization.\n");
        exit(EXIT_FAILURE);
    }

    png_init_io(*png_ptr, fp);
    png_set_sig_bytes(*png_ptr, 8);
    png_read_info(*png_ptr, *info_ptr);

    *width = png_get_image_width(*png_ptr, *info_ptr);
    *height = png_get_image_height(*png_ptr, *info_ptr);
    *color_type = png_get_color_type(*png_ptr, *info_ptr);
    *bit_depth = png_get_bit_depth(*png_ptr, *info_ptr);

    png_read_update_info(*png_ptr, *info_ptr);

    *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * (*height));
    for (int y = 0; y < *height; y++) {
        (*row_pointers)[y] = (png_byte *)malloc(png_get_rowbytes(*png_ptr, *info_ptr));
    }

    png_read_image(*png_ptr, *row_pointers);

    fclose(fp);
}

void generate_random_filename(char *buffer, size_t buffer_size, const char *extension) {
    struct timeval time_now;
    gettimeofday(&time_now, NULL);
    srand((unsigned int)(time_now.tv_usec));

    int random_number = rand();  // Generate a random number
    snprintf(buffer, buffer_size, "output/decrypt/file_%d.%s", random_number, extension);
}

void decode_data_from_png(const char *png_filename) {
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep *row_pointers;
    int width, height, color_type, bit_depth;

    read_png_file(png_filename, &row_pointers, &png_ptr, &info_ptr, &width, &height, &color_type, &bit_depth);

    // Extract embedded extension with a maximum limit
    char extension[10] = {0};  // Buffer limited to 10 characters for extensions
    int ext_index = 0;
    int bit_index = 0;

    for (int y = 0; y < height && ext_index < sizeof(extension) - 1; y++) {
        png_bytep row = row_pointers[y];
        for (int x = 0; x < width * 3 && ext_index < sizeof(extension) - 1; x++) {
            extension[ext_index] |= (row[x] & 1) << (7 - bit_index);
            bit_index++;
            if (bit_index == 8) {
                bit_index = 0;
                ext_index++;
            }
        }
    }

    extension[ext_index] = '\0';  // Null terminate the string

    // Check for empty or invalid extensions
    if (ext_index == 0 || strlen(extension) == 0) {
        fprintf(stderr, "Error: Failed to extract a valid file extension.\n");
        exit(EXIT_FAILURE);
    }

    // Debug: print the extracted extension
    printf("Extracted file extension: %s\n", extension);

    // Extract file data
    unsigned char *file_data = (unsigned char *)malloc(width * height * 3);
    if (file_data == NULL) {
        perror("Failed to allocate memory for file data.");
        exit(EXIT_FAILURE);
    }

    int data_index = 0;
    bit_index = 0;

    for (int y = 0; y < height && data_index < width * height * 3; y++) {
        png_bytep row = row_pointers[y];
        for (int x = 0; x < width * 3 && data_index < width * height * 3; x++) {
            file_data[data_index] |= (row[x] & 1) << (7 - bit_index);
            bit_index++;
            if (bit_index == 8) {
                bit_index = 0;
                data_index++;
            }
        }
    }

    // Ensure data_index is valid
    if (data_index == 0) {
        fprintf(stderr, "Error: Failed to extract file data.\n");
        free(file_data);
        exit(EXIT_FAILURE);
    }

    // Generate random filename with extracted extension
    char output_path[512];
    generate_random_filename(output_path, sizeof(output_path), extension);

    // Save the extracted data
    FILE *output_file = fopen(output_path, "wb");
    if (!output_file) {
        perror("Error opening output file for writing");
        free(file_data);
        return;
    }

    fwrite(file_data, 1, data_index, output_file);
    fclose(output_file);

    // Cleanup
    free(file_data);
    printf("Data successfully decrypted and saved as %s.\n", output_path);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <png>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    create_output_directories();
    decode_data_from_png(argv[1]);

    return 0;
}
