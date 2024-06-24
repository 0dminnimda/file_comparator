#include "memmap.h"

#include <stdio.h>

#ifdef _WIN32

#include <windows.h>

int mmap_open(const char *path, MemoryMappedFile *mmf) {
    void *file;
    void *mapping;
    void *ptr;
    uint64_t file_size;

    // Open the file
    file = CreateFile(
        path,
        GENERIC_READ,                          // dwDesiredAccess
        0,                                     // dwShareMode
        NULL,                                  // lpSecurityAttributes
        OPEN_EXISTING,                         // dwCreationDisposition
        FILE_ATTRIBUTE_NORMAL,                 // dwFlagsAndAttributes
        0                                      // hTemplateFile
    );
    if (file == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateFile failed with error %ld\n", GetLastError());
        return 1;
    }

    // Get the size of the file
    if (!GetFileSizeEx(file, (LARGE_INTEGER*)&file_size)) {
        fprintf(stderr, "GetFileSize failed with error %ld\n", GetLastError());
        CloseHandle(file);
        return 1;
    }
    if (file_size == 0) {
        mmf->ptr = NULL;
        mmf->length = 0;
        mmf->data_.windows.mapping = 0;
        mmf->data_.windows.file = INVALID_HANDLE_VALUE;
        CloseHandle(file);
        return 0;
    }

    // Create a file mapping object
    mapping = CreateFileMapping(
        file,
        NULL,                          // Mapping attributes
        PAGE_READONLY,                 // Protection flags
        0,                             // MaximumSizeHigh
        0,                             // MaximumSizeLow
        NULL                           // Name
    );
    if (mapping == 0) {
        fprintf(stderr, "CreateFileMapping failed with error %ld\n", GetLastError());
        CloseHandle(file);
        return 1;
    }

    // Map the view of the file
    ptr = MapViewOfFile(
        mapping,
        FILE_MAP_READ,         // dwDesiredAccess
        0,                     // dwFileOffsetHigh
        0,                     // dwFileOffsetLow
        0                      // dwNumberOfBytesToMap
    );  
    if (ptr == NULL) {
        fprintf(stderr, "MapViewOfFile failed with error %ld\n", GetLastError());
        CloseHandle(mapping);
        CloseHandle(file);
        return 1;
    }

    mmf->ptr = ptr;
    mmf->length = file_size;
    mmf->data_.windows.mapping = mapping;
    mmf->data_.windows.file = file;
    return 0;
}

void mmap_close(MemoryMappedFile *mmf) {
    UnmapViewOfFile(mmf->ptr);
    CloseHandle(mmf->data_.windows.mapping);
    CloseHandle(mmf->data_.windows.file);
}

#else // POSIX-like systems

#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

int mmap_open(const char *path, MemoryMappedFile *mmf) {
    int fd;
    void *ptr;
    uint64_t file_size;

    // Open the file
    fd = open(FILEPATH, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Error opening file: %d (%s)", errno, strerror(errno));
        return 1;
    }

    // Get the size of the file
    {
        struct stat st;
        if (fstat(fd, &st) == -1) {
            fprintf(stderr, "Error getting file size: %d (%s)", errno, strerror(errno));
            return 1;
        }
        file_size = st.st_size;
    }
    if (file_size == 0) {
        mmf->ptr = NULL;
        mmf->length = 0;
        mmf->data_.linux.file_descriptor = -1;
        return 0;
    }

    // Memory-map the file
    ptr = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (ptr == MAP_FAILED) {
        close(fd);
        fprintf(stderr, "Error mapping the file: %d (%s)", errno, strerror(errno));
        return 1;
    }

    mmf->ptr = ptr;
    mmf->length = file_size;
    mmf->data_.linux.file_descriptor = fd;
    return 0;
}

void mmap_close(MemoryMappedFile *mmf) {
    munmap(mmf->ptr, mmf->length);
    close(mmf->dat_.linux.file_descriptor);
}

#endif

