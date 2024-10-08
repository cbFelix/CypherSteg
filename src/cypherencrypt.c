#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Create output directories if they don't exist
void create_output_directories() {
    struct stat st = {0};

    // Create output/encrypt if it does not exist
    if (stat("output/encrypt", &st) == -1) {
        mkdir("output", 0700);
        mkdir("output/encrypt", 0700);
    }
}

// Function to read the PNG file
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

// Function to write the PNG file
void write_png_file(const char *filename, png_bytep *row_pointers, png_structp png_ptr, png_infop info_ptr, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Error opening file for writing PNG");
        exit(EXIT_FAILURE);
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fprintf(stderr, "Failed to create png_struct.\n");
        exit(EXIT_FAILURE);
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        fprintf(stderr, "Failed to create png_info.\n");
        exit(EXIT_FAILURE);
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Error during PNG writing.\n");
        exit(EXIT_FAILURE);
    }

    png_init_io(png_ptr, fp);

    png_set_IHDR(
        png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);

    fclose(fp);
}

// Function to embed metadata and file data into PNG
void encode_data_in_png(const char *png_filename, const char *file_to_hide) {
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep *row_pointers;
    int width, height, color_type, bit_depth;

    read_png_file(png_filename, &row_pointers, &png_ptr, &info_ptr, &width, &height, &color_type, &bit_depth);

    // Open the file to hide
    FILE *file_to_embed = fopen(file_to_hide, "rb");
    if (!file_to_embed) {
        perror("Error opening file to hide");
        return;
    }

    // Get file name and extension
    char *filename = strrchr(file_to_hide, '/');
    filename = (filename) ? filename + 1 : (char *)file_to_hide;

    // Extract file extension
    char *extension = strrchr(filename, '.');
    if (!extension) {
        extension = "";
    } else {
        extension++;
    }

    // Get file size
    fseek(file_to_embed, 0, SEEK_END);
    long file_size = ftell(file_to_embed);
    fseek(file_to_embed, 0, SEEK_SET);

    unsigned char *file_data = (unsigned char *)malloc(file_size);
    fread(file_data, 1, file_size, file_to_embed);

    // Embed file size, name, and extension
    int extension_length = strlen(extension);
    int data_index = 0, bit_index = 0;

    // First, embed extension length and extension (max 255 bytes for simplicity)
    for (int y = 0; y < height && data_index < extension_length; y++) {
        png_bytep row = row_pointers[y];
        for (int x = 0; x < width * 3 && data_index < extension_length; x++) {
            row[x] = (row[x] & ~1) | ((extension[data_index] >> (7 - bit_index)) & 1);
            bit_index++;
            if (bit_index == 8) {
                bit_index = 0;
                data_index++;
            }
        }
    }

    // Then embed the file data
    bit_index = 0;
    data_index = 0;
    for (int y = 0; y < height && data_index < file_size; y++) {
        png_bytep row = row_pointers[y];
        for (int x = 0; x < width * 3 && data_index < file_size; x++) {
            row[x] = (row[x] & ~1) | ((file_data[data_index] >> (7 - bit_index)) & 1);
            bit_index++;
            if (bit_index == 8) {
                bit_index = 0;
                data_index++;
            }
        }
    }

    // Create the output path
    char output_path[512];
    snprintf(output_path, sizeof(output_path), "output/encrypt/%s.png", filename);

    // Write the new PNG with embedded data
    write_png_file(output_path, row_pointers, png_ptr, info_ptr, width, height);

    // Cleanup
    free(file_data);
    fclose(file_to_embed);
    printf("Data successfully encrypted in the PNG image.\n");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <png> <file_to_hide>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    create_output_directories();
    encode_data_in_png(argv[1], argv[2]);

    return 0;
}
