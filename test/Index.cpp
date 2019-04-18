#include <iostream>
#include <cstdlib>

#include "Index.h"
#include "vector.h"
#include "ISR.h"

using namespace std;

int main( int argc, char** argv )
   {
   if ( argc < 2 || argc > 5 )
      {
      cerr << "Usage: ./bin/index <numPosts> [ <postingStart> <word> <-r> ]" << endl;
      exit( 1 );
      }

   Index* index = Index::GetIndex( );

   unsigned long long posting = ( argc == 2 ? 1 : strtoull( argv[ 2 ], nullptr, 10 ) );
   //bool randomized = argc > 4;
   const char* word = argc > 3 ? argv[ 3 ] : "butter";

   Vector< unsigned long long > postings;

   for ( unsigned numPosts = strtoul( argv[ 1 ], nullptr, 10 ); numPosts > 0; numPosts-- )
      postings.push_back( posting++ );
      //postings.push_back( posting += randomized ? rand( ) % 256 : 1 );
         
   index->AddPostings( word, &postings );

   String wordString( word );
   WordISR wordISR( wordString );

   cout << wordISR.NextInstance( 18000 ) << endl;
   }
