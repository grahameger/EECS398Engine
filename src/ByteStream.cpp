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


void OutputByteStream::AddByte( const unsigned char byte )
   {
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
      {
      bitMask = 128;
      output->AddByte( curByte );
      curByte = 0;
      }
   }


