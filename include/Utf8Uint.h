#ifndef UTF8UINT_H
#define UTF8UINT_H

#include "ByteStream.h"

class PairUtf8Uint;

// A wrapper for numbers that follow a UTF-8 style encoding.
//
// This means that numbers take up a variable number of bytes proportional to
//  the size of their value. (Numbers like 0-255 can be represented in one byte,
//  whereas larger numbers make take two or more)
// The advantage of this type of number comes in disk storage. We no longer have
//  to allocate space for the largest possible value. If we have values between
//  0 and 4,294,967,296, we no longer have to allocate 64 bits per number just
//  because our largest value is > 2^32.


// This class stores just a regular Utf8Uint. It can be anywhere between 0
//  and 18,446,744,073,709,551,615. 
//
// The Utf8Uint can be broken up into bytes using the following pattern:
//
//    [Unary Preamble of byte size - 1] 0 [Integer data]
//
//  meaning that small numbers like 7 have the following footprint:
//
//    0000 0111
//    (No unary preamble, a 0, and the 7 bits that translate to 7)
//
//  and the largest numbers (like the max above) have a footprint of:
//
//    1111 1111 0000 0000 1111 1111 1111 1111
//    1111 1111 1111 1111 1111 1111 1111 1111
//    1111 1111 1111 1111
//    (A unary preamble of 8, meaning the number takes 9 bytes), a 0,
//    (the 9 bytes that translate to 18,446,744,073,709,551,615)
//
class Utf8Uint
   {
   public:
      Utf8Uint( );
      Utf8Uint( unsigned long long value );

      const unsigned long long GetValue( ) const;
      void Reset( );

   private:
      unsigned long long value;

   friend PairUtf8Uint;
   friend InputByteStream& operator>> ( InputByteStream& byteStream, Utf8Uint& number );
   friend OutputByteStream& operator<< ( OutputByteStream& byteStream, Utf8Uint& number );
   };

// Read in a Utf8Uint from a byteStream
InputByteStream& operator>> ( InputByteStream& byteStream, Utf8Uint& number );

// Write a Utf8Uint to a byteStream
OutputByteStream& operator<< ( OutputByteStream& byteStream, Utf8Uint& number );

#endif
