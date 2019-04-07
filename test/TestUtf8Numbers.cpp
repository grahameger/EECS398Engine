#include <iostream>
#include "Utf8Uint.h"
#include "PairUtf8Uint.h"
#include "ByteStream.h"

using namespace std;

int main( )
   {
   Utf8Uint int1;
   PairUtf8Uint intPair;

   String numbers("\x02\x81\x01\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 14);
   String numbers2("\x82\x01\x02\xFF\x80\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", 23);

   InputByteStream bs(numbers);
   InputByteStream bs2(numbers2);
   for ( int i = 0; i < 3; i++ )
      {
      bs >> int1;
      cout << int1.GetValue() << endl;
      }

   for ( int i = 0; i < 3; i++ )
      {
      bs2 >> intPair;
      cout << intPair.GetFirst() << " " << intPair.GetSecond() << endl;
      }

   }
