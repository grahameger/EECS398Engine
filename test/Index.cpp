#include <iostream>
#include "Index.h"
#include "PostingList.h"

using namespace std;

void PrintPL( PostingList* pl, unsigned size );

int main( )
   {
   Index index( ( char* )"testIndex", 128, 3 );

   /*
   cout << index->NumBlocks( ) << endl;

   auto subBlock = index->GetPostingList( "butter" );
   cout << subBlock.GetBlockIndex( ) << " " << ( unsigned )subBlock.GetSubBlockIndex( ) << endl;

   PostingList pl;
   if ( subBlock.GetInitialized( ) )
      cout << "Already Initialized PostingList" << endl;
   else
      cout << "Uninitialized PostingList" << endl;

   pl.AddPosting( 1 );
   pl.AddPosting( 7 );
   pl.AddPosting( 92 );
   pl.AddPosting( 12'804 );
   pl.AddPosting( 65'536 );

   if ( pl.GetByteSize( ) <= subBlock.GetBlockSize( ) )
      {
      cout << "Update in place. Adding " << pl.GetByteSize( ) << " bytes to a space of " << subBlock.GetBlockSize( ) << " bytes." << endl;
      StringView subBlockString = subBlock.GetStringView( );
      //pl.UpdateInPlace( subBlockString );
      }
   */
   }


void PrintPL( PostingList* pl, unsigned size )
   {
   StringView data = pl->GetData( size );

   for ( unsigned i = 0; i < size; i++ )
      cout << hex << ( unsigned )( unsigned char )data.CString( )[ i ] << " ";
   cout << dec << endl;
   }
