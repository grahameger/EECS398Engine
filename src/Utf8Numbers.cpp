#include "Utf8Uint.h"
#include "PairUtf8Uint.h"
#include "ByteStream.h"

void PrintNumberBytes( OutputByteStream& byteStream, unsigned long long number, 
      int nonPreambleBytes );
int PrintPreambleBytes( OutputByteStream& byteStream, 
      unsigned long long number );
bool GetIsPair( InputByteStream& byteStream, unsigned long long& postPreamble,
      int& numBytesLeft );
unsigned long long GetNumber( InputByteStream& byteStream, 
      unsigned long long curValue, int& numBytesLeft );
int GetUnaryPreamble( InputByteStream& byteStream, 
      unsigned long long& postPreamble );


// #######################
// ### CLASS FUNCTIONS ###
// #######################


Utf8Uint::Utf8Uint( ) : value( 0 )
   { }

Utf8Uint::Utf8Uint( unsigned long long value ) : value( value )
   { }

PairUtf8Uint::PairUtf8Uint( ) : first( 0 ), second( )
   { }

const unsigned long long Utf8Uint::GetValue( ) const
   {
   return value;
   }

void Utf8Uint::Reset( )
   {
   value = 0;
   }

const unsigned long long PairUtf8Uint::GetFirst( ) const
   {
   return first;
   }

const unsigned long long PairUtf8Uint::GetSecond( ) const
   {
   return second.GetValue( );
   }

void PairUtf8Uint::Reset( )
   {
   first = 0;
   second.value = 1;
   }


InputByteStream& operator>> ( InputByteStream& byteStream, Utf8Uint& number )
   {
   number.Reset( );

   // The data in the same byte as the end of the preamble
   unsigned long long curValue;

   int numBytesLeft = GetUnaryPreamble( byteStream, curValue );

   number.value = GetNumber( byteStream, curValue, numBytesLeft );

   return byteStream;
   }


InputByteStream& operator>> ( InputByteStream& byteStream, PairUtf8Uint& number )
   {
   number.Reset( );

   // The data in the same byte as the end of the preamble
   unsigned long long curValue;
   // Num bytes left in the first number
   int numBytesLeft;

   // Get isPair and the Preamble
   bool isPair = GetIsPair( byteStream, curValue, numBytesLeft );
   if ( curValue == 256 )
      {
      curValue = 0;
      numBytesLeft += GetUnaryPreamble( byteStream, curValue );
      }

   // Read the number portion of the first number
   number.first = GetNumber( byteStream, curValue, numBytesLeft );

   // Read the second number if it is there
   if ( isPair )
      byteStream >> number.second;

   return byteStream;
   }


OutputByteStream& operator<< ( OutputByteStream& byteStream, Utf8Uint number )
   {
   unsigned nonPreambleBytes = PrintPreambleBytes( byteStream, number.GetValue( ) );
   PrintNumberBytes( byteStream, number.GetValue( ), nonPreambleBytes );
   return byteStream;
   }


OutputByteStream& operator<< ( OutputByteStream& byteStream, PairUtf8Uint number )
   {
   // Add pair bit
   byteStream.GetBitIterator( ).AddBit( number.GetSecond( ) != 1 );

   // Add first number
   unsigned nonPreambleBytes = PrintPreambleBytes( byteStream, number.GetFirst( ) );
   PrintNumberBytes( byteStream, number.GetFirst( ), nonPreambleBytes );

   // Add second number
   if ( number.GetSecond( ) != 1 )
      byteStream << number.second;

   return byteStream;
   }


// ########################
// ### HELPER FUNCTIONS ###
// ########################


void PrintNumberBytes( OutputByteStream& byteStream, unsigned long long number, 
      int nonPreambleBytes )
   {
   // If nonPreambleBytes is 8, there is weird behavior with bit shifting?
   unsigned char remainder = 0;
   if ( nonPreambleBytes < 8 )
      remainder = number >> ( 8 * nonPreambleBytes-- );
   else
      nonPreambleBytes--;
   auto& bitIterator = byteStream.GetBitIterator( );

   // If the rest of the preamble byte is just padding, flush it
   if ( remainder == 0 )
      bitIterator.Flush( );

   // Otherwise, put the remainder into the current byte
   else
      {
      while ( remainder != 0 )
         {
         unsigned char currentDivisor = 1 << ( bitIterator.BitsLeft( ) - 1 );
         if ( currentDivisor <= remainder )
            {
            remainder -= currentDivisor;
            bitIterator.AddBit( 1 );
            }
         else
            bitIterator.AddBit( 0 );
         }
      bitIterator.Flush( );
      }

   // Add the rest of the number, byte by byte
   for ( ; nonPreambleBytes >= 0; nonPreambleBytes-- )
      {
      unsigned char curByte = ( number >> ( 8 * nonPreambleBytes ) ) % 256;
      byteStream.AddByte( curByte );
      }
   }


// "Print" to byteStream just the unary preamble bits of this number. Ensure that upon
//  completion, the bitIterator for byteStream will begin writing after the preamble.
//
int PrintPreambleBytes( OutputByteStream& byteStream, unsigned long long number )
   {
   int nonPreambleBytes = 0;

   // Add preamble 1s
   auto& bitIterator = byteStream.GetBitIterator( );
   while ( number > 127 )
      {
      // Add a 1
      bitIterator.AddBit( 1 );
      // Shift the number to check
      number >>= 8;
      // Increment the number of nonPreambleBytes
      nonPreambleBytes++;
      }

   // Get bitsLeft in curNumber
   int bitsLeft = 7;
   while ( ( ( unsigned )1 << --bitsLeft ) > number && bitsLeft > 0 ) { }

   // If it can't fit in the current byte, add another 1
   if ( bitIterator.BitsLeft( ) - 1 < bitsLeft + 1 )
      {
      // Add a 1
      bitIterator.AddBit( 1 );
      // Increment the number of nonPreambleBytes
      nonPreambleBytes++;
      }

   // Add preamble 0
   bitIterator.AddBit( 0 );

   return nonPreambleBytes;
   }


// Return whether the data for this PairUtf8Uint is for just one or two numbers.
//  Ensure that numBytesLeft is equal to whatever portion of the preamble is in
//  the byte with our pair flag, and that postPreamble is 256 if the preamble
//  doesn't end in this byte and the number after the preamble in this byte if
//  it does.
//
bool GetIsPair( InputByteStream& byteStream, unsigned long long& postPreamble, 
      int& numBytesLeft )
   {
   unsigned char byte = byteStream.GetNextByte( );
   bool isPair = byte > 127;

   switch ( byte & 127 )
      {
      // All 1s after bool
      case 127:
         numBytesLeft = 7;
         postPreamble = 256;
         break;

      // Preamble ends in bit 7
      case 126:
         numBytesLeft = 6;
         postPreamble = 0;
         break;

      // Preamble ends in bits 1-6
      default:
         numBytesLeft = 0;
         for ( unsigned char bitMask = 64; bitMask != 1; bitMask /= 2 )
            {
            // bit at bitMask is 0
            if ( !( byte & bitMask ) )
               {
               postPreamble = byte & (bitMask - 1);
               return isPair;
               }

            numBytesLeft += 1;
            } // for
         break;
      } // switch

   return isPair;
   }


// Return the number from the data portion of the Utf8Uint as calculated by the
//  current value post preamble (curValue) and the number of bytes left in the
//  number (numBytesLeft)
//
unsigned long long GetNumber( InputByteStream& byteStream, 
      unsigned long long curValue, int& numBytesLeft )
   {
   unsigned long long returnVal = 0;

   for ( ; numBytesLeft > 0; numBytesLeft-- )
      {
      // Add curValue shifted to it's correct size
      returnVal += curValue << numBytesLeft * 8;
      curValue = byteStream.GetNextByte( );
      }

   returnVal += curValue;
   return returnVal;
   }


// Return the unary preamble of a Utf8Uint and set postPreamble to the data in the
//  last read byte after the preamble's terminating 0
//
// Ex:
//    1111 1111 1100 1001 (has a preamble of 10, and a postPreamble of 9)
//
int GetUnaryPreamble( InputByteStream& byteStream, 
      unsigned long long& postPreamble )
   {
   int numBytesLeft = 0;
   unsigned char byte;

   while( true )
      {
      switch( byte = byteStream.GetNextByte( ) )
         {
         // All 1s
         case 255:
            numBytesLeft += 8;
            break;

         // 7 1s and a 0 (end of preamble)
         case 254:
            numBytesLeft += 7;
            postPreamble = 0;
            return numBytesLeft;

         // Preamble ends in bytes 0-6
         // (xxxxxxxy) where at least one x = 0
         default:
            for ( unsigned char bitMask = 128; bitMask != 1; bitMask /= 2 )
               {
               // bit at bitMask is 0
               if ( !( byte & bitMask ) )
                  {
                  postPreamble = byte & (bitMask - 1);
                  return numBytesLeft;
                  }

               numBytesLeft += 1;
               } // for
            break;

         } // switch
      } // while

   return 0;
   }


