// Created by Graham Eger on 4/1/2019

#include "PersistentBitVector.h"
#include <cstdio>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

PersistentBitVector::PersistentBitVector(String filename) {
    static const size_t SIZE_READ_SIZE = sizeof(size_t);
    // check if the file exists
    struct stat buffer;
    bool fileExists = (stat(filename.CString(), &buffer) == 0);

    // different behavior here for these two
    if (fileExists) {
        // open up the file
        fd = open(filename.CString(), O_RDWR);
        if (fd < 0) {
            fprintf(stderr, "open() returned -1 - error: %s\n", strerror(errno));
            // TODO: more error handling
        }
        // read the size from the start of the file
        size_t tempSize = 0;
        ssize_t rv = pread(fd, &tempSize, SIZE_READ_SIZE, 0);
        if (rv != SIZE_READ_SIZE) {
            fprintf(stderr, "error initializing PersistentBitVector: pread %s\n", strerror(errno));
            // TODO: more error handling
        }
        size_t realFileSize = sizeof(SizeAndLock) + tempSize;
        // memory map the requested file size and save it as the base pointer
        base = mmap(nullptr, realFileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (base == (void *)-1) {
            fprintf(stderr, "error initializing PersistentBitVector: mmap %s\n", strerror(errno));
            // TODO: more error handling
        }

        // setup our pointers
        sizeAndLock = (SizeAndLock*)base;
        dataBase = (uint8_t*)base + sizeof(SizeAndLock);
        
        // reset the size and run the rw-lock constructor
        memset((void*)sizeAndLock, 0x0, sizeof(sizeAndLock));
        sizeAndLock->dataSize = tempSize;
        sizeAndLock->rwLock = threading::ReadWriteLock();
    } else {
        fd = open(filename.CString(), O_RDWR | O_CREAT);
        if (fd < 0) {
            fprintf(stderr, "open() returned -1 - error: %s\n", strerror(errno));
            // TODO: more error handling
        }
        // set the initial size
        size_t realFileSize = sizeof(SizeAndLock) + DEFAULT_SIZE;
        // memory map the requested file size and save it as the base pointer
        base = mmap(nullptr, realFileSize, PROT_READ | PROT_WRITE, MAP_SHARED)
    }
}