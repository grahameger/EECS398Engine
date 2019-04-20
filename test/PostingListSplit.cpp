#include <iostream>
#include "PostingList.h"
#include "vector.h"

using namespace std;

void TestPostingList( Vector< unsigned long long > postings, 
      Vector< unsigned > blockSizes );
void PrintPL( PostingList* pl, unsigned size );

int main( )
   {
   TestPostingList( { 3, 5, 19, 20, 4012, 4277, 65536 }, { 32, 24 } );
   }


void TestPostingList( Vector< unsigned long long > postings, 
      Vector< unsigned > blockSizes )
   {
   PostingList pl;

   for ( unsigned i = 0; i < postings.size( ); i++ )
      pl.AddPosting( postings[ i ] );

   cout << "Original PostingList: " << pl.GetByteSize( ) << " bytes." << endl;
   PrintPL( &pl, pl.GetByteSize( ) );
   
   for ( unsigned i = 0; i < blockSizes.size( ); i++ )
      {
      auto splitPl = pl.Split( blockSizes[ i ] );
      cout << "\tSplit into blocks of size: " << blockSizes[ i ] << " bytes." << endl;
      
      for ( unsigned j = 0; j < splitPl.size( ); j++ )
         {
         cout << "\t";
         PrintPL( splitPl[ j ], blockSizes[ i ] );
         }

      cout << endl;
      }
   }


void PrintPL( PostingList* pl, unsigned size )
   {
   StringView data = pl->GetData( size );

   for ( unsigned i = 0; i < size; i++ )
      cout << hex << ( unsigned )( unsigned char )data.CString( )[ i ] << " ";
   cout << dec << endl;
   }
