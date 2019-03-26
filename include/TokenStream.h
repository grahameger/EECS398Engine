#ifndef TOKENSTREAM_H
#define TOKENSTREAM_H

#include "List.h"

class TokenStream 
   {
public:
   TokenStream( const char* filename );
   ~TokenStream( );

   TokenStream( const TokenStream& ) = delete;
   void operator=( const TokenStream& ) = delete;

<<<<<<< HEAD
	void DiscardWhitespace();

	bool MatchKeyword(const String& keyword);
	bool MatchEndline();
	String MatchPath();

	bool MatchNextKeyword(const String& keyword);
	bool MatchNextEndline();
=======
   void DiscardWhitespace( );

   bool MatchKeyword( const String& keyword );
   bool MatchEndline( );
   String MatchPath( );
>>>>>>> 54dcc0655e3d85c1c787f08437c044c9ba94ff07

   bool MatchNextKeyword( const String& keyword );
   bool MatchNextEndline( );

   operator bool( ) const;

private:
<<<<<<< HEAD
	const static int BufferSize;

	int fileDescriptor, lexemeStart, peekIndex, lastIndex;
	List<char*> buffers;
	List<char*>::Iterator front, back;

	void AddPage();
	void ResetPeek();
	void DecrementPeek();
	void ConsumeLexeme();
	String ConsumeLexemeToString();
	const int PeekNext();
};
=======
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
>>>>>>> 54dcc0655e3d85c1c787f08437c044c9ba94ff07

#endif
