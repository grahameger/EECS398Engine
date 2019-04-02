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
    bool get(size_t idx);
    void set(size_t idx, bool b);
    void resize(size_t newSize);
    size_t size();
private:
    int fd;
    static const size_t DEFAULT_SIZE = 16;
    // byte order shouldn't matter here?
    static constexpr const uint8_t SET_BITS[] = {
        0b10000000,
        0b01000000,
        0b00100000,
        0b00010000,
        0b00001000,
        0b00000100,
        0b00000010,
        0b00000001,
    };
    struct Header {
        size_t dataSize;
        threading::ReadWriteLock rwLock;
    };
    uint8_t * data;
    Header * header;
};

#endif