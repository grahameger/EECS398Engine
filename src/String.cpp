#include <cstring>
#include "String.h"

const char* String::nullString = "";


String::String( ) : cstring( nullptr ), size( 0 ) { }


String::String( const char* toCopy ) : size( strlen( toCopy ) )
   {
   cstring = new char[ size + 1 ];
   strcpy( cstring, toCopy );
   }


String::String( char*&& toMove ) : cstring( toMove ), 
      size( strlen( toMove ) )
   {
   toMove = nullptr;
   }


String::String( const String& toCopy )
   {
   size = toCopy.size;

   if ( toCopy.cstring == nullptr )
      {
	  cstring = nullptr;
	  return;
	  }
   
   cstring = new char[ size + 1 ];
   strcpy( cstring, toCopy.cstring );
   }


String::String( String&& toMove )
   {
   size = toMove.size;
   cstring = toMove.cstring;
   toMove.cstring = nullptr;
   }


String& String::operator = ( const String& toCopy )
   {
   String temp( toCopy );
   Swap( temp );
   return *this;
   }


String& String::operator = ( String&& toMove )
   {
   Swap( toMove );
   return *this;
   }


String::~String( )
   {
   delete[ ] cstring;
   cstring = nullptr;
   }


void String::Swap( String& toSwap )
   {
   char* tempCString = cstring;
   int tempSize = size;
   cstring = toSwap.cstring;
   size = toSwap.size;
   toSwap.cstring = tempCString;
   toSwap.size = tempSize;
   }


bool String::Empty( ) const { return size == 0; }


int String::Size( ) const { return size; }


const char* String::CString( ) const
   {
   return cstring == nullptr ? nullString : cstring;
   }


bool String::Compare( const String& other ) const
   {
   if ( size != other.size ) return false;

   int index = 0, i;
   while ( ( i = index++ ) != size && cstring[ i ] == other.cstring[ i ]) { }

   return index > size;
   }


const char String::operator[ ] ( int index ) const
   {
   if ( cstring == nullptr )
	  return nullString[ index ];

   return cstring[ index ];
   }


char& String::operator[ ] ( int index )
   {
   if ( cstring == nullptr )
      return nullString[ index ];
   
   return cstring[ index ];
   }


String& operator+= ( const String& rhs )
   {
   if ( rhs.cstring == nullptr )
      return *this;

   int newSize = size + rhs.size;
   char* newCString = new char[newSize + 1];

   if ( cstring != nullptr )
      strcpy( newCString, cstring );
   strcpy( newCString + size, rhs.cstring );

   delete cstring;
   cstring = newCString;
   size = newSize;

   return *this;
   }


String::operator bool( ) const
   {
   return size > 0;
   }
