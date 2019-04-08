#include "ByteStream.h"


InputByteStream::InputByteStream( const String& toRead, bool forwards )
      : reading( toRead ), byteNum( 0 ), forwardStream( forwards )
   {
   if ( !forwardStream )
      byteNum = reading.Size( ) - 1;
   }


const unsigned char InputByteStream::GetNextByte( )
   {
   const unsigned char returnVal = reading[ byteNum ];
   if ( forwardStream )
      byteNum++;
   else
      byteNum--;

   return returnVal;
   }


OutputByteStream::OutputByteStream( bool forwards ) 
      : writing( 63 ), byteNum( 0 ), forwardStream( forwards )
   {
   if ( !forwardStream )
      byteNum = 63;
   }


const String& OutputByteStream::GetString( ) const
   {
   return writing;
   }


// Warning: Here there be sketchy math
const String OutputByteStream::HexString( ) const
   {
   // Size is bytenum * 2 
   //    (2 characters in hex per byte)
   // Plus bytenum / 2
   //    (one space every two bytes)
   int size = byteNum * 2 + byteNum / 2;
   String hexString( size );

   for ( int i = 0; i < size; i++ )
      {
      // If a spot for a space
      if ( i > 0 && ( i + 1 ) % 5 == 0 )
         {
         hexString[ i ] = ' ';
         continue;
         }

      // The adjusted index into writing (curIndex - seen spaces)
      int j = i - (i / 5);

      // Used to get the current nybble of the String writing
      //    writing[ j / 2 ] gives us the current byte
      //    Then we shift it right by 4 if we want the first
      //    nybble (j % 2 == 0), and by 0 otherwise.
      //    Then we take the result of the shift mod 16 in order
      //    to isolate the nybble in the case where we shift by 0
      //
      unsigned char curNybble = 
            ( ( unsigned char ) writing[ j / 2 ] >> ( 4 * !( j % 2 ) ) ) % 16;

      // Convert the nybble to a character, 0-9 or A-F
      if ( curNybble < 10 )
         hexString[ i ] = '0' + curNybble;
      else
         hexString[ i ] = 'A' + (curNybble - 10);
      }

   return hexString;
   }


void OutputByteStream::AddByte( const unsigned char byte )
   {
   if ( currentIterator )  
      currentIterator->Flush( );

   writing[ byteNum ] = byte;

   if ( forwardStream && ++byteNum == writing.Size( ) )
      writing.Allocate( writing.Size( ) + 1 );
   else if ( !forwardStream && --byteNum == -1 )
      writing.Allocate( writing.Size( ) + 1, false );
   }


OutputByteStream::BitIterator& OutputByteStream::GetBitIterator( )
   {
   if ( !currentIterator )
      currentIterator = new BitIterator( this );

   return *currentIterator;
   }


OutputByteStream::BitIterator::BitIterator( OutputByteStream* out )
      : curByte( 0 ), bitMask( 128 ), output( out )
   { }


void OutputByteStream::BitIterator::AddBit( bool bit )
   {
   if ( bit )
      curByte |= bitMask;

   bitMask >>= 1;

   if ( bitMask == 0 )
      Flush( );
   }


void OutputByteStream::BitIterator::Flush( )
   {
   if ( bitMask == 128 )
      return;

   bitMask = 128;
   output->AddByte( curByte );
   curByte = 0;
   }


unsigned char OutputByteStream::BitIterator::BitsLeft( )
   {
   switch ( bitMask )
      {
      case 128:
         return 8;
      case 64:
         return 7;
      case 32:
         return 6;
      case 16:
         return 5;
      case 8:
         return 4;
      case 4:
         return 3;
      case 2:
         return 2;
      case 1:
         return 1;
      }
   return 0;
   }


