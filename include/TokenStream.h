#pragma once
#ifndef TOKENSTREAM_H_398
#define TOKENSTREAM_H_398

#include "String.h"
#include "List.h"

class TokenStream 
   {
public:
   TokenStream( const char* filename );
   ~TokenStream( );

   TokenStream( const TokenStream& ) = delete;
   void operator=( const TokenStream& ) = delete;

   void DiscardWhitespace( );

   bool MatchKeyword( const String& keyword );
   bool MatchEndline( );
   String MatchPath( );

   bool MatchNextKeyword( const String& keyword );
   bool MatchNextEndline( );

   operator bool( ) const;

private:
   const static int BufferSize;

   int fileDescriptor, lexemeStart, peekIndex, lastIndex;
   List<char*> buffers;
   List<char*>::Iterator front, back;

   void AddPage( );
   void ResetPeek( );
   void DecrementPeek( );
   void ConsumeLexeme( );
   String ConsumeLexemeToString( );
   const int PeekNext( );
   };

#endif
