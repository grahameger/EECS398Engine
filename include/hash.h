// Created by Graham Eger on 4/2/2019
// Graham Eger added a customization for std::hash<String> on 04/08/2019

#pragma once
#ifndef EECS_398_HASH_FUNCTIONS_H
#define EECS_398_HASH_FUNCTIONS_H

#include <stdint.h>
#include <cstddef>
#include <string>
#include "String.h"

namespace hash {
    // templated Murmur64A
    template <typename T> struct Hash {
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
        // we're okay with overflow here
        // just a simple summing hash function
        static uint64_t sum(const T& t) {
            size_t sum = 0;
            for (size_t i = 0; i < sizeof(T); i++) {
                sum += ((uint8_t*)&t)[i];
            }
            return sum;
        }
    };
    template <> struct Hash<char*> {
        // double hashing the string with
        // djb2 multiply and xor version
        static uint64_t get(const char * s)
        {
            uint32_t h1 = 5831;
            uint32_t h2 = 5831;
            int c;
            while ((c = *s++)) {
                h1 = ((h1 << 5) + h1) + c;
                h2 = (((h2) << 5) + h2) ^ c;
            }
            return (((uint64_t)h1) << 32) + h2;
        }
        uint64_t operator()(const char * s)
        {
            return get(s);
        }
    };

    template <> struct Hash<String> {
        static uint64_t get(const String& str) {
            return Hash<char*>{}.get(str.CString());
        }
        uint64_t operator()(const String& str)
        {
            return Hash<char*>{}.get(str.CString());
        }
    };

    // specialization for any buffer, size at runtime
    // doesn't need to be a null terminated string like djb2
    // use T for compile time lengths
    // this isn't really useful for use in any data structures
    // that follow STL paradigms but could be useful for something else
    template <> struct Hash<uint8_t*> {
        static uint64_t get(const uint8_t* ptr, const size_t len)
        {
            uint32_t h1 = 5831;
            uint32_t h2 = 5831;
            const uint8_t * end = ptr + len;
            for (; ptr != end; ++ptr) 
            {
                h1 = ((h1 << 5) + h1) + (*ptr);
                h2 = (((h2) << 5) + h2) ^ (*ptr);
            }
            return (((uint64_t)h1) << 32) + h2;
        }
    };
    template <> struct Hash<std::string> {
        static uint64_t get(const std::string& str) {
            return Hash<char*>{}.get(str.c_str());
        }
    };
}

namespace std {
    template<> 
    struct hash<String> {
        std::size_t operator()(const String &s) const {
            return ::hash::Hash<char*>{}(s.CString());
        }
    };
}

#endif