// Created by Graham Eger on 4/1/2019

#include "mmap.h"
#include <cstdio>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

static const size_t PAGE_SIZE = getpagesize();

// just wrap mmap and do the error checking here
void * mmapWrapper(int fd, size_t size, size_t offset) {
    offset = roundUp(offset, PAGE_SIZE);
    extendFile(fd, size + offset);
    void * rv = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
    if (rv == (void*)-1) {
        fprintf(stderr, "error mmap'ing %zu bytes at offset %zu - mmap %s", size, offset, strerror(errno));
        exit(1);
    }
    return rv;
}

int munmapWrapper(void * addr, size_t size) {
    int rv = munmap(addr, size);
    if (rv == -1) {
        fprintf(stderr, "error destructing PersistentBitVector: munmap %s\n", strerror(errno));
        exit(1);
    }
    return rv;
}

size_t fileSize(int fd) {
    struct stat buffer;
    fstat(fd, &buffer);
    return buffer.st_size;
}

void extendFile(int fd, size_t newSize) {
    newSize = roundUp(newSize, PAGE_SIZE);
    if (newSize == 0) {
        newSize = PAGE_SIZE;
    }
    if (fileSize(fd) < newSize) {
       ftruncate(fd, newSize);
    }
}

size_t roundUp(size_t numToRound, size_t multiple) {
    if (multiple == 0)
        return numToRound;
    size_t remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;
    return numToRound + multiple - remainder;
}