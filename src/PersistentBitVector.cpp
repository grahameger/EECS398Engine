// Created by Graham Eger on 4/1/2019

#include "PersistentBitVector.h"
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cassert>
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
    if (header->dataSize == 0) {
        header->rwLock = threading::ReadWriteLock();
    }
    header->rwLock.writeLock();
    if (!fileExists) {
        header->dataSize = DEFAULT_SIZE_BYTES;
    }

    // mmap the data portion
    // the data size should be the number of bits specified, not bytes
    data = (uint8_t*)mmapWrapper(fd, header->dataSize, sizeof(Header));
    // zero out the page? seriously why is this not working?
    memset(data, 0x0, header->dataSize);
    header->rwLock.unlock();
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

bool PersistentBitVector::at(size_t idx) {
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
    size_t byteToAccess = idx / 8;
    size_t bitToAccess = idx % 8;
    uint8_t byteToAdd = SET_BITS[bitToAccess];
    
    data[byteToAccess] += byteToAdd;
    header->rwLock.unlock();
}

void PersistentBitVector::resize(size_t newSize) {
    // grab the write lock
    header->rwLock.writeLock();
    if (newSize * 8 > header->dataSize * 8) {
        // unmap data
        munmapWrapper(data, header->dataSize);
        // remap data with the new size and sizeof(Header) offset
        data = (uint8_t*)mmapWrapper(fd, newSize / 8, sizeof(Header));

        // zero out the newly mapped region
        size_t half = header->dataSize;
        memset(this->data + half, 0x0, half);

        header->dataSize = newSize / 8;
    }
    header->rwLock.unlock();
}

size_t PersistentBitVector::size() {
    header->rwLock.readLock();
    size_t rv = header->dataSize * 8;
    header->rwLock.unlock();
    return rv; 
}