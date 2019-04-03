// Created by Graham Eger on 4/2/2019

#pragma once
#ifndef EECS_398_HASH_FUNCTIONS_H
#define EECS_398_HASH_FUNCTIONS_H

#include <stdint.h>
#include <cstddef>

// templated Murmur64A
namespace hash {
template <typename T> struct Hash
    {
    static uint64_t get(const T& t)
    {
        const uint64_t seed = 0x8009e2d374011cb9;
        const uint64_t m = 0xc6a4a7935bd1e995;
        const int32_t r = 47;
        const size_t len = sizeof(T);
        const uint64_t * data = (const uint64_t *) &t;
        const uint64_t * end = (len >> 3) + data;
        
        uint64_t h = seed ^ (sizeof(T) * m);
        
        while (data != end)
        {
            uint64_t k = *data++;
            k *= m;
            k ^= k >> r;
            k *= m;
            h ^= k;
            h *= m;
        }
        
        const uint8_t * data2 = (const uint8_t *) &t;
        
        switch (len & 7)
        {
            case 7: h ^= (uint64_t)(data2[6]) << 48;
            case 6: h ^= (uint64_t)(data2[5]) << 40;
            case 5: h ^= (uint64_t)(data2[4]) << 32;
            case 4: h ^= (uint64_t)(data2[3]) << 24;
            case 3: h ^= (uint64_t)(data2[2]) << 16;
            case 2: h ^= (uint64_t)(data2[1]) << 8;
            case 1: h ^= (uint64_t)(data2[0]);
                h *= m;
        };
        
        h ^= h >> r;
        h *= m;
        h ^= h >> r;
        
        return h;
    }
    };
}

#endif