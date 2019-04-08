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


const String OutputByteStream::HexString( ) const
   {
   int size = byteNum - 1;
   String hexString( size * 2 );

   for ( int i = 0; i < size * 2; i++ )
      {

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


