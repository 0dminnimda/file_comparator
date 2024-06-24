#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memmap.h"

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define BUFFER_SIZE 128 * 1024
#define MIN_BYTES_BETWEEN_PROGRESS BUFFER_SIZE * 8 * 8  // every 8MiB
#define FRACTION_COUNT 50

typedef uint64_t u64;

int report_progress(u64 pos, u64 *last_pos, u64 total_size, char *progress_str) {
    if (pos == 0 || pos == total_size || pos - *last_pos > MIN_BYTES_BETWEEN_PROGRESS) {
        *last_pos = pos;

        double fraction_f = ((double)pos / total_size * FRACTION_COUNT);
        size_t fraction_i = fraction_f;
        for (size_t i = 0; i < fraction_i; ++i) {
            progress_str[i] = '#';
        }

        printf("\rProgress: [%.*s] %.2lf%% (%zu/%zu)", FRACTION_COUNT, progress_str, fraction_f * (100 / FRACTION_COUNT), pos, total_size);
        fflush(stdout); // Ensure the progress bar is displayed correctly

        return 1;
    }
    return 0;
}

int compare_via_memmap(const char *file_path_1, const char *file_path_2) {
    MemoryMappedFile file1;
    if (mmap_open(file_path_1, &file1)) {
        fprintf(stderr, "Error opening file %s\n", file_path_1);
        return 1;
    }

    MemoryMappedFile file2;
    if (mmap_open(file_path_2, &file2)) {
        fprintf(stderr, "Error opening file %s\n", file_path_2);
        mmap_close(&file1);
        return 1;
    }

    if (file1.length != file2.length) {
        printf("Warning: File sizes differ %zu != %zu.\n", file1.length, file2.length);
    }
    u64 total_size = min(file1.length, file2.length);

    char progress_str[FRACTION_COUNT];
    memset(progress_str, ' ', FRACTION_COUNT);

    u64 differences = 0;
    u64 last_pos = 0;
    u64 i = 0;
    int reported = 0;
    for (; i < total_size; ++i) {
        reported |= report_progress(i, &last_pos, total_size, progress_str);

        if (file1.ptr[i] != file2.ptr[i]) {
            fprintf(stderr, "%.*sDifference found at position %zu: byte 1=%02X, byte 2=%02X\n", reported, "\n", i, file1.ptr[i], file2.ptr[i]);
            differences++;
            reported = 0;
        }
    }

    report_progress(total_size, &last_pos, total_size, progress_str);
    printf("\n");  // move to new line after progress

    printf("Number of differences found: %llu\n", differences);

    mmap_close(&file1);
    mmap_close(&file2);

    return differences > 0;
}

u64 get_file_size(FILE *file, const char *file_path) {
    u64 size = 0;
    fseek(file, 0, SEEK_END);
    if (fgetpos(file, (fpos_t*)&size)) {
        printf("Warning: Size of file '%s' is unknown\n", file_path);
    }
    fseek(file, 0, SEEK_SET);
    return size;
}

int compare_via_buffed_fread(const char *file_path_1, const char *file_path_2) {
    FILE *file1 = fopen(file_path_1, "rb");
    if (!file1) {
        fprintf(stderr, "Error opening file %s\n", file_path_1);
        return 1;
    }

    FILE *file2 = fopen(file_path_2, "rb");
    if (!file2) {
        fprintf(stderr, "Error opening file %s\n", file_path_2);
        fclose(file1);
        return 1;
    }

    u64 size1 = get_file_size(file1, file_path_1);
    u64 size2 = get_file_size(file2, file_path_2);
    if (size1 != size2) {
        printf("Warning: File sizes differ %zu != %zu.\n", size1, size2);
    }
    u64 total_size = min(size1, size2);

    char *buffer = malloc(BUFFER_SIZE * 2);
    char *buffer1 = buffer;
    char *buffer2 = buffer1 + BUFFER_SIZE;

    char progress_str[FRACTION_COUNT];
    memset(progress_str, ' ', FRACTION_COUNT);

    u64 differences = 0;
    u64 pos = 0;
    u64 last_pos = 0;
    int reported = 0;
    while (pos < total_size && !feof(file1) && !feof(file2)) {
        reported |= report_progress(pos, &last_pos, total_size, progress_str);

        size_t read1 = fread(buffer1, sizeof(char), BUFFER_SIZE, file1);
        size_t read2 = fread(buffer2, sizeof(char), BUFFER_SIZE, file2);
        size_t read = min(read1, read2);

        for (size_t i = 0; i < read; ++i) {
            if (buffer1[i] != buffer2[i]) {
                fprintf(stderr, "%.*sDifference found at position %zu: byte 1=%02X, byte 2=%02X\n", reported, "\n", pos, buffer1[i], buffer2[i]);
                differences++;
                reported = 0;
            }
        }

        pos += read;
    }

    report_progress(total_size, &last_pos, total_size, progress_str);
    printf("\n");  // move to new line after progress

    printf("Number of differences found: %llu\n", differences);

    free(buffer);

    fclose(file1);
    fclose(file2);

    return differences > 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <file1> <file2>\n", argv[0]);
        fflush(stdout);
        exit(1);
    }

    return compare_via_buffed_fread(argv[1], argv[2]);
    // return compare_via_memmap(argv[1], argv[2]);
}

