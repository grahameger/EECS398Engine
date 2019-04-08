// Created by Graham Eger on 04/06/2019

#pragma once
#ifndef BLOOM_398_H
#define BLOOM_398_H

#include <cstddef>
#include <cstring>
#include "hash.h"


// not really going to attempt to make it thread safe, if there are false positives that's fine
// since we just go to the filesystem anyways
template <typename Key>
class BloomFilter {
public:
    BloomFilter(size_t n) : dataSize(n), numElements(0) {
        data = (uint8_t*)calloc(dataSize, sizeof(uint8_t));
    }
    ~BloomFilter() {
        free(data); data = nullptr;
        dataSize = 0;
    }
    BloomFilter(const BloomFilter& other) {
        data = (uint8_t*)calloc(other.dataSize, sizeof(uint8_t));
        dataSize = other.dataSize;
        numElements = other.numElements;
        std::memcpy(data, other.data, other.dataSize);
    }

    // Checks whether an element probably exists in the set, or definitely doesn't.
    bool exists(Key k) {
        size_t h1 = hash::Hash<Key>{}.get(k) % dataSize; 
        size_t h2 = hash::Hash<Key>{}.get(k + static_cast<char>(39)) % dataSize; 
        size_t h3 = hash::Hash<Key>{}.get(k + static_cast<char>(198)) % dataSize; 
        size_t h4 = hash::Hash<Key>{}.get(k + static_cast<char>(171)) % dataSize; 
        size_t h5 = hash::Hash<Key>{}.get(k + static_cast<char>(270)) % dataSize; 
        size_t h6 = hash::Hash<Key>{}.get(k + static_cast<char>(150)) % dataSize; 
        size_t h7 = hash::Hash<Key>{}.get(k + static_cast<char>(181)) % dataSize;
        return isBitSet(h1) && isBitSet(h2) && isBitSet(h3) && isBitSet(h4) &&
            isBitSet(h5) && isBitSet(h6) && isBitSet(h7);
    }

    size_t bufferSize() {
        return dataSize;
    }
    void add(Key k) {
        // 7 hash functions
        size_t h1 = hash::Hash<Key>{}.get(k) % dataSize; 
        size_t h2 = hash::Hash<Key>{}.get(k + static_cast<char>(39)) % dataSize; 
        size_t h3 = hash::Hash<Key>{}.get(k + static_cast<char>(198)) % dataSize; 
        size_t h4 = hash::Hash<Key>{}.get(k + static_cast<char>(171)) % dataSize; 
        size_t h5 = hash::Hash<Key>{}.get(k + static_cast<char>(270)) % dataSize; 
        size_t h6 = hash::Hash<Key>{}.get(k + static_cast<char>(150)) % dataSize; 
        size_t h7 = hash::Hash<Key>{}.get(k + static_cast<char>(181)) % dataSize;
        // set all the bits
        setBit(h1);
        setBit(h2);
        setBit(h3);
        setBit(h4);
        setBit(h5);
        setBit(h6);
        setBit(h7);
        numElements++;
    }
    void clear() {
        memset((void*)data, 0x0, dataSize);
        numElements = 0;
    }
private:
    void setBit(size_t idx) {
        data[idx / 8] |= 1UL << (idx % 8);
    }
    bool isBitSet(size_t idx) {
        return (data[idx / 8] >> (idx % 8) & 1U);
    }
    uint8_t * data;
    size_t dataSize;
    size_t numElements;
};


#endif