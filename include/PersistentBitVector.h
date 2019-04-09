// Created by Graham Eger on 4/1/2019

#pragma once
#ifndef PERSISTENTBITVECTOR_H_398
#define PERSISTENTBITVECTOR_H_398

#include <cstdlib>
#include <cstdint>
#include "String.h"
#include "threading.h"


// File Structure for a PersistentBitVector file.
// [Size: 64-bit unsigned int][ReadWriteLock 56 bytes][]

class PersistentBitVector {
public:
    PersistentBitVector(String filename);
    ~PersistentBitVector();
    PersistentBitVector(const PersistentBitVector&) = delete;
    bool at(size_t idx);
    void set(size_t idx, bool b);
    void resize(size_t newSize);
    size_t size();
private:
    int fd;
    static const size_t DEFAULT_SIZE_BYTES = 8;

    // byte order shouldn't matter here?
    struct Header {
        size_t dataSize;
        pthread_mutex_t mutex;
    };
    uint8_t * data;
    Header * header;
};

#endif