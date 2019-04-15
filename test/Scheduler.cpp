#include <iostream>
#include "IndexScheduler.h"

using namespace std;

int main( )
   {
   IndexScheduler::IndexFilename = "testIndex";
   IndexScheduler* index = IndexScheduler::GetScheduler( 128, 3, 3 );
   cout << index->NumBlocks( ) << endl;

   auto subBlock = index->GetPostingList( "butter" );
   cout << subBlock.GetBlockIndex( ) << " " << ( unsigned )subBlock.GetSubBlockIndex( ) << endl;
   }
