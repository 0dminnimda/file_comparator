#pragma once

#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include <stdint.h>

typedef struct {
    uint8_t *ptr;
    uint64_t length;

    union {
        struct {
            void *file;
            void *mapping;
        } windows;
        struct {
            int file_descriptor;
        } linux;
    } data_;
} MemoryMappedFile;

int mmap_open(const char *path, MemoryMappedFile *mmf);
void mmap_close(MemoryMappedFile *mmf);

#endif // MEMORY_MAP_H

