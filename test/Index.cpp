#include <iostream>
#include "Index.h"
#include "PostingList.h"
#include "vector.h"

using namespace std;

int main( int argc, char** argv )
   {
   if ( argc < 2 || argc > 3 )
      {
      cerr << "Usage: ./bin/index <numPosts> [ <postingStart> ]" << endl;
      exit( 1 );
      }

   Index index( ( char* )"testIndex" );

   unsigned long long posting = ( argc == 2 ? 1 : strtoull( argv[ 2 ], nullptr, 10 ) );

   Vector< unsigned long long > postings;

   for ( unsigned numPosts = strtoul( argv[ 1 ], nullptr, 10 ); numPosts > 0; numPosts-- )
      postings.push_back( posting++ );
         
   index.AddPostings( "butter", &postings );
   }
