#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define BUFFER_SIZE 128 * 1024
#define MIN_BYTES_BETWEEN_PROGRESS BUFFER_SIZE * 8 * 8  // every 8MiB
#define FRACTION_COUNT 150

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <file1> <file2>\n", argv[0]);
        exit(1);
    }

    FILE *file1 = fopen(argv[1], "rb");
    FILE *file2 = fopen(argv[2], "rb");

    if (!file1) {
        printf("Error opening file %s\n", argv[1]);
        exit(1);
    }

    if (!file2) {
        printf("Error opening file %s\n", argv[2]);
        exit(1);
    }

    fseek(file1, 0, SEEK_END);
    long size1 = ftell(file1);
    rewind(file1);

    fseek(file2, 0, SEEK_END);
    long size2 = ftell(file2);
    rewind(file2);

    if (size1 != size2) {
        printf("Warning: File sizes differ.\n");
    }

    size_t total_size = min(size1, size2);

    int differences = 0;

    char *buffer1 = malloc(BUFFER_SIZE);
    char *buffer2 = malloc(BUFFER_SIZE);
    size_t pos = 0;
    size_t last_pos = -MIN_BYTES_BETWEEN_PROGRESS;

    char progress_str[FRACTION_COUNT];
    memset(progress_str, ' ', FRACTION_COUNT);
    size_t last_progress = 0;

    while (!feof(file1) && !feof(file2)) {
        if (pos == 0 || pos - last_pos > MIN_BYTES_BETWEEN_PROGRESS) {
            last_pos = pos;

            size_t fraction = ((double)pos / total_size * FRACTION_COUNT);
            for (size_t i = last_progress; i < fraction; ++i) {
                progress_str[i] = '#';
            }

            printf("\rProgress: [%.*s] (%zu/%zu)", FRACTION_COUNT, progress_str, pos, total_size);
            fflush(stdout); // Ensure the progress bar is displayed correctly
        }

        size_t read1 = fread(buffer1, sizeof(char), BUFFER_SIZE, file1);
        size_t read2 = fread(buffer2, sizeof(char), BUFFER_SIZE, file2);
        size_t read = min(read1, read2);

        for (size_t i = 0; i < read; ++i) {
            if (buffer1[i] != buffer2[i]) {
                fprintf(stderr, "Difference found at position %zu: byte 1=%02X, byte 2=%02X\n", pos + i, buffer1[i], buffer2[i]);
                differences++;
            }
        }

        pos += read;
    }

    printf("Number of differences found: %d\n", differences);

    free(buffer1);
    free(buffer2);

    fclose(file1);
    fclose(file2);

    return differences > 0;
}

