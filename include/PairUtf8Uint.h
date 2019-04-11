#ifndef PAIRUTF8UINT_H
#define PAIRUTF8UINT_H

#include "Utf8Uint.h"


// A wrapper for numbers that follow a UTF-8 style encoding.
//
// This means that numbers take up a variable number of bytes proportional to
//  the size of their value. (Numbers like 0-255 can be represented in one byte,
//  whereas larger numbers make take two or more)
// The advantage of this type of number comes in disk storage. We no longer have
//  to allocate space for the largest possible value. If we have values between
//  0 and 4,294,967,296, we no longer have to allocate 64 bits per number just
//  because our largest value is > 2^32.


// This class stores either a single Utf8Uint, or a pair of them. The first
//  bit is used as a flag to denote which variation is being used.
//
// The two encodings are as follows:
//
//    1 [Unary preamble of first # size] 0 [First Integer data] [Utf8UInt second #]
//    0 [Unary preamble of # size] 0 [Integer data]
//  
//  where the first 1 denotes that this data is a pair of integers, and the 0 means
//  that it is just a single integer.
//
class PairUtf8Uint
   {
   public:
      PairUtf8Uint( );

      const unsigned long long GetFirst( ) const;
      const unsigned long long GetSecond( ) const;
      void Reset( );

   private:
      unsigned long long first;
      Utf8Uint second;

   friend InputByteStream& operator>> ( InputByteStream& byteStream, PairUtf8Uint& number );
   friend OutputByteStream& operator<< ( OutputByteStream& byteStream, PairUtf8Uint number );
   };

// Read in a PairUtf8Uint from a byteStream
InputByteStream& operator>> ( InputByteStream& byteStream, PairUtf8Uint& number );

// Write a PairUtf8Uint to a byteStream
OutputByteStream& operator<< ( OutputByteStream& byteStream, PairUtf8Uint number );

#endif
