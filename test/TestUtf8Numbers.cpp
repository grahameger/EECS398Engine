#include <iostream>
#include "Utf8Uint.h"
#include "PairUtf8Uint.h"
#include "ByteStream.h"

using namespace std;

int main( )
   {
   Utf8Uint int1;
   PairUtf8Uint intPair;

   // Forward string with Utf8Uints of value:
   // 2, 257, 18.446.744.073.709.551.615, and 22.516.072.924.689
   String numbers( "\x02\x81\x01\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFC\x14\x7A\x6E\x87\x46\x11", 20 );

   // Forward string with PairUtf8Uints of value:
   // (2, 1) [Formatted incorrectly], (2, 1), and (18,446,744,073,709,551,615, 18,446,744,073,709,551,615)
   String numbers2( "\x82\x01\x02\xFF\x80\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 23 );

   // Backward string with Utf8Uints of value:
   // 2, 257, 18.446.744.073.709.551.615, and 22.516.072.924.689
   String numbers3( "\x11\x46\x87\x6E\x7A\x14\xFC\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\xFF\x01\x81\x02", 20 );

   // Backward string with PairUtf8Uints of value:
   // (2, 1) [Formatted incorrectly], (2, 1), and (18,446,744,073,709,551,615, 18,446,744,073,709,551,615)
   String numbers4( "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x80\xFF\x02\x01\x82", 23 );

   InputByteStream is( numbers );
   OutputByteStream os;
   for ( int i = 0; i < 4; i++ )
      {
      is >> int1;
      cout << int1.GetValue( ) << endl;
      os << int1;
      cout << os.HexString( ).CString( ) << endl << endl;
      }

   InputByteStream is2( numbers2 );
   OutputByteStream os2;
   for ( int i = 0; i < 3; i++ )
      {
      is2 >> intPair;
      cout << intPair.GetFirst( ) << " " << intPair.GetSecond( ) << endl;
      os2 << intPair;
      cout << os2.HexString( ).CString( ) << endl << endl;
      }

   InputByteStream is3( numbers3, false );
   OutputByteStream os3( false );
   for ( int i = 0; i < 4; i++ )
      {
      is3 >> int1;
      cout << int1.GetValue( ) << endl;
      os3 << int1;
      cout << os3.HexString( ).CString( ) << endl << endl;
      }

   InputByteStream is4( numbers4, false );
   OutputByteStream os4( false );
   for ( int i = 0; i < 3; i++ )
      {
      is4 >> intPair;
      cout << intPair.GetFirst( ) << " " << intPair.GetSecond( ) << endl;
      os4 << intPair;
      cout << os4.HexString( ).CString( ) << endl << endl;
      }

   }
