#include <iostream>
#include "PostingList.h"

using namespace std;

int main( )
   {
   PostingList postingList;                        // 16 bytes (2 unsigned, 1 unsigned long long)
   postingList.AddPosting( 1 );                    // 3 bytes (2 for index, 1 for posts)
   postingList.AddPosting( 127 );                  // 1 byte (1 for posts)
   postingList.AddPosting( 256 );                  // 2 byte (2 for posts 256 - 127 = 129 > 128)
   cout << postingList.GetByteSize( ) << endl;     // 22 bytes

   StringView postingListString = postingList.GetData( 32 );
   for ( int i = 0; i < 32; i++ )
      cout << hex << ( unsigned )( unsigned char )postingListString[ i ] << " ";
   cout << dec << endl << endl;

   PostingList postingList2( postingListString );
   postingList2.AddPosting( 257 );                 // 1 byte (1 for posts)
   postingList2.AddPosting( 65536 );               // 7 bytes (3 for posts, 4 for index) c0 fe ff, and 05 00 00 c1
   cout << postingList2.GetByteSize( ) << endl;    // 30 bytes

   StringView postingListString2 = postingList2.GetData( 32 );
   for ( int i = 0; i < 32; i++ )
      cout << hex << ( unsigned )( unsigned char )postingListString2[ i ] << " ";
   cout << dec << endl << endl;

   postingList2.UpdateInPlace( postingListString );
   for ( int i = 0; i < 32; i++ )
      cout << hex << ( unsigned )( unsigned char )postingListString[ i ] << " ";
   cout << dec << endl << endl;
   }
