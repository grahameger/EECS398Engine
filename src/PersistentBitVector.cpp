// Created by Graham Eger on 4/1/2019

#include "PersistentBitVector.h"
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "mmap.h"


// TODO: reduce code duplication
PersistentBitVector::PersistentBitVector(String filename) {
    // check if the file exists
    struct stat buffer;
    bool fileExists = (stat(filename.CString(), &buffer) == 0);
    
    // open file with correct flags
    int openFlags = O_RDWR;
    if (!fileExists) {
        openFlags |= O_CREAT;
    }
    fd = open(filename.CString(), openFlags);
    if (fd < 0) {
        fprintf(stderr, "open() returned -1 - error: %s\n", strerror(errno));
        // TODO: more error handling
    }

    // mmap and setup the header portion
    header = (Header*)mmapWrapper(fd, sizeof(Header), 0);
    header->rwLock = threading::ReadWriteLock();
    if (!fileExists) {
        header->dataSize = DEFAULT_SIZE;
    }

    // mmap the data portion
    data = (uint8_t*)mmapWrapper(fd, header->dataSize, sizeof(Header));
}

// close and unmap the file with a write lock
PersistentBitVector::~PersistentBitVector() {
    // get a write lock so that all reads and writes finish up
    header->rwLock.writeLock();
    // 2 unmaps
    munmapWrapper(data, header->dataSize);
    // we reset the lock on a reconstruction of this data structure so shouldn't be a problem
    munmapWrapper(header, sizeof(Header));
    close(fd);
}

bool PersistentBitVector::get(size_t idx) {
    // no bounds checking, there's a O(1) .size() operator if the
    // programmer wants to use it.
    // dataBase[idx / 8] returns the byte which contains the bit we want to return
    // we do a bitwise AND with it to just return the singular bit specified by idx
    header->rwLock.readLock();
    bool rv = data[idx / 8] & (SET_BITS[idx % 8]);
    header->rwLock.unlock();
    return rv;
}

void PersistentBitVector::set(size_t idx, bool b) {
    // same as get() but summing instead of a bitwise AND to save the value
    header->rwLock.writeLock();
    data[idx / 8] += (SET_BITS[idx % 8]);
    header->rwLock.unlock();
}

void PersistentBitVector::resize(size_t newSize) {
    // grab the write lock
    header->rwLock.writeLock();
    if (newSize > header->dataSize) {
        // unmap data
        munmapWrapper(data, header->dataSize);
        // remap data with the new size and sizeof(Header) offset
        data = (uint8_t*)mmapWrapper(fd, newSize, sizeof(Header));
    }
    header->rwLock.unlock();
}