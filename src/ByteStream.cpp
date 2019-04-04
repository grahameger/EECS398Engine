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
