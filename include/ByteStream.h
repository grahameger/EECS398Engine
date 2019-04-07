#ifndef BYTESTREAM_H
#define BYTESTREAM_H

#include "String.h"

// This class is given a String which it then turns into a ByteStream. The goal
//  is to make a simple wrapper that abstracts whether the string is being read
//  forwards or backwards.
//
class InputByteStream
   {
   public:
      InputByteStream(const String& toRead, bool forwards = true);

      const unsigned char GetNextByte();
      InputByteStream& operator>> ( const unsigned char& character );
      InputByteStream& operator>> ( int& number );

   private:
      const String reading;
      int byteNum;
      bool forwardStream;

   };

// This class is creating a String from a ByteStream. The goal is to make a simple
//  wrapper that abstracts whether the string is being read forwards or backwards.
//
class OutputByteStream
   {
   public:
      OutputByteStream(bool forwards = true);

      const String& GetString( );

      void AddByte( const unsigned char byte );
      OutputByteStream& operator<< ( const unsigned char character );
      OutputByteStream& operator<< ( const int number );

      class BitIterator
         {
         public:
            void AddBit( bool bit );

         private: 
            unsigned char curByte;
            unsigned char bitMask;
            OutputByteStream* output;

            BitIterator( OutputByteStream* out );

            friend OutputByteStream;

         };

      BitIterator& GetBitIterator( );

   private:
      String writing;
      int byteNum;
      bool forwardStream;
      BitIterator* currentIterator;

   };

#endif
