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
    bool get(size_t idx);
    void set(bool b);

    void resize(size_t newSize);
private:
    static const size_t DEFAULT_SIZE = 16;
    struct SizeAndLock {
        size_t dataSize;
        threading::ReadWriteLock rwLock;
    };

    static const size_t StructSize = sizeof(SizeAndLock);

    int fd;
    void * base;
    uint8_t * dataBase;
    SizeAndLock * sizeAndLock;
};

#endif