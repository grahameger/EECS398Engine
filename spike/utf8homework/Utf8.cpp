//
//  Utf8.cpp
//  utf8-practice
//
//  Created by Graham Eger on 2/1/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include "Utf8.h"
#include <string.h>
#include <string>
#include <locale>

// used for a static lookup table
// for each byte in a possible sequence
typedef struct {
   uint8_t mask;
   uint8_t lead;
   uint32_t beg;
   uint32_t  end;
   size_t bits_stored;
} utf_t;

// utf[0] is the infomation for a
static utf_t utf[] = {
   /* mask        lead        beg      end       bits */
   {0b00111111, 0b10000000, 0,       0,        6    },
   {0b01111111, 0b00000000, 0000,    0177,     7    },
   {0b00011111, 0b11000000, 0200,    03777,    5    },
   {0b00001111, 0b11100000, 04000,   0177777,  4    },
   {0b00000111, 0b11110000, 0200000, 04177777, 3    },
   {0},
};

// SizeOfUTF8 tells the number of bytes it will take to encode the
// specified value.

// SizeOfUTF8( GetUtf8( p ) ) does not tell how many bytes encode
// the character pointed to by p because p may point to a malformed
// character.
// Assuming machine byte order at this point.
size_t SizeOfUtf8(Unicode c) {
   if (c < 0x7f) {
      return 1;
   } else {
      return 2;
   }
   return 1;
}

// IndicatedLength looks at the first byte of a Utf8 sequence
// and determines the expected length.  Useful for avoiding
// buffer overruns when reading Utf8 characters.  Return 1
// for an invalid first byte.
size_t IndicatedLength( const Utf8 *p ) {
   switch (__builtin_clz(~(*p))) {
      case 0:
         return 1;
      case 1:
         return 1;
      case 2:
         return 2;
      case 3:
         return 3;
      case 4:
         return 4;
   }
   return 1;
}


// Get the UTF-8 character as a Unicode value.
// If it's an invalid UTF-8 encoding for a U-16
// character, return the special malformed
// character code.
Unicode GetUtf8( const Utf8 *p ) {
   size_t bytes = IndicatedLength(p);
   if (bytes > 2)
      return ReplacementCharacter;
   size_t shift = utf[0].bits_stored * (bytes - 1);
   Unicode codep = (*p++ & utf[bytes].mask) << shift;
   for (auto i = 0; i < bytes; ++i, ++p) {
      shift -= utf[0].bits_stored;
      codep |= ((char)*p & utf[0].mask) << shift;
   }
   return codep;
}

// Bounded version.  bound = one past last valid
// byte.  bound == 0 means no bounds checking.
// If a character runs past the last valid byte,
// return the replacement character.
// same code after bounds check
Unicode GetUtf8( const Utf8 *p, const Utf8 *bound ) {
   size_t bytes = IndicatedLength(p);
   if (bytes > bound - p || bytes > 2) {
      return ReplacementCharacter;
   }
   size_t shift = utf[0].bits_stored * (bytes - 1);
   Unicode codep = (*p++ & utf[bytes].mask) << shift;
   for (auto i = 0; i < bytes; ++i, ++p) {
      shift -= utf[0].bits_stored;
      codep |= ((char)*p & utf[0].mask) << shift;
   }
   return codep;
}

// NextUtf8 will determine the length of the Utf8 character at p
// by examining the first byte, then scan forward that amount,
// stopping early if it encounters an invalid continuation byte
// or the bound, if specified.
const Utf8 *NextUtf8( const Utf8 *p, const Utf8 *bound) {
   size_t bytes = IndicatedLength(p);
   int len = 0;
   for (utf_t *; <#condition#>; <#increment#>) {
      <#statements#>
   }
}
