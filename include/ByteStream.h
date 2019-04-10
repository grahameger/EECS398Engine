#ifndef BYTESTREAM_H
#define BYTESTREAM_H

#include "String.h"
#include "StringView.h"

// This class is given a String which it then turns into a ByteStream. The goal
//  is to make a simple wrapper that abstracts whether the string is being read
//  forwards or backwards.
//
class InputByteStream
   {
   public:
      InputByteStream(const String& toRead, bool forwards = true);

      const unsigned char GetNextByte();

      // TODO: Add Iterator like below for setting bit by bit

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
      ~OutputByteStream( );

      const StringView GetString( ) const;
      // For Debugging
      const String HexString( ) const;

      void AddByte( const unsigned char byte );

      class BitIterator
         {
         public:
            void AddBit( bool bit );
            void Flush( );

            unsigned char BitsLeft( );

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
