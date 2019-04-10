//
//  main.cpp
//  BloomFilter
//
//  Created by Graham Eger on 2/11/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#include <iostream>
#include <functional>

// templated Murmur64A
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

template <> struct Hash<char*>
{
   // double hashing the string with
   // djb2 multiply and xor version
   static uint64_t get(const char * s)
   {
      uint32_t h1 = 5831;
      uint32_t h2 = 5831;
      int c;
      while ((c = *s++)) {
         h1 = ((h1 << 5) + h1) + c;
         h2 = ((h2) << 5) + h2 ^ c;
      }
      return (((uint64_t)h1) << 32) + h2;
   }
};


int main(int argc, const char * argv[]) {
   static Hash<uint64_t> hasher;
   static Hash<char*> strHasher;
   std::cout << hasher.get(45345) % (2 << 10) << '\n';
   std::cout << strHasher.get("Hello world!\n") << '\n';
   return 0;
}
