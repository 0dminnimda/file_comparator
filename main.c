#include <stdio.h>
#include <stdlib.h>

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define BUFFER_SIZE 256

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

    {
        fseek(file1, 0, SEEK_END);
        long size1 = ftell(file1);
        rewind(file1);

        fseek(file2, 0, SEEK_END);
        long size2 = ftell(file2);
        rewind(file2);

        if (size1 != size2) {
            printf("Warning: File sizes differ.\n");
        }

    }

    int differences = 0;

    char buffer1[BUFFER_SIZE];
    char buffer2[BUFFER_SIZE];
    size_t pos = 0;
    while (!feof(file1) && !feof(file2)) {
        size_t read1 = fread(buffer1, sizeof(char), BUFFER_SIZE, file1);
        size_t read2 = fread(buffer2, sizeof(char), BUFFER_SIZE, file2);
        size_t read = min(read1, read2);

        for (long i = 0; i < read; ++i) {
            if (buffer1[i] != buffer2[i]) {
                fprintf(stderr, "Difference found at position %zu: byte 1=%02X, byte 2=%02X\n", pos + i, buffer1[i], buffer2[i]);
                differences++;
            }
        }

        pos += read;
    }

    fclose(file1);
    fclose(file2);

    printf("Number of differences found: %d\n", differences);

    return differences > 0;
}

