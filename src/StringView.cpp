#include "StringView.h"

StringView::StringView( const char* CString, int length, bool forwards )
      : CString( CString ), length( length ), forwards( forwards )
   { }


bool StringView::Empty( ) const
   {
   return length == 0;
   }


int StringView::Size( ) const
   {
   return length;
   }


const char StringView::operator[ ] ( int index ) const
   {
   int newIndex = forwards ? index : ( length - index );
   return CString[ newIndex ];
   }


bool StringView::Compare( const StringView& other ) const
   {
   if ( length != other.length )
      return false;

   for ( int i = 0; i < length; i++ )
      {
      int newIndex = forwards ? i : ( length - i );
      if ( CString[ newIndex ] != other[ i ] )
         return false;
      }

   return true;
   }


StringView::operator bool( ) const
   {
   return length > 0;
   }


