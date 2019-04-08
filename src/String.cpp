#include <iostream>
#include <cstring>
#include <cctype>
#include "String.h"

const char* String::nullString = "";


String::String( const int length ) : cstring( nullptr ), size( length )
   {
   if ( size > 0 )
      {
      cstring = new char[ size + 1 ];
      cstring[ size ] = 0;
      }
   }


String::String( const char* toCopy, int length ) : size( length )
   {
   if ( length == -1 )
      size = strlen( toCopy );
   cstring = new char[ size + 1 ];
   cstring[ size ] = 0;
   memcpy( cstring, toCopy, size );
   }


String::String( char*&& toMove, int length ) : cstring( toMove ), 
      size( length )
   {
   toMove = nullptr;
   if ( length == -1 )
      size = strlen( cstring );
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
   memcpy( cstring, toCopy.cstring, size );
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
   if ( cstring == nullptr ) {
      cstring = new char[1];
      cstring[0] = 0x0;
      return cstring[index];
   }
   return cstring[ index ];
   }


String& String::operator+= ( const String& rhs )
   {
   if ( rhs.cstring == nullptr )
      return *this;

   int newSize = size + rhs.size;
   char* newCString = new char[newSize + 1];

   if ( cstring != nullptr )
      memcpy( newCString, cstring, size );
   memcpy( newCString + size, rhs.cstring, rhs.size );

   delete cstring;
   cstring = newCString;
   size = newSize;

   return *this;
   }


String::operator bool( ) const
   {
   return size > 0;
   }


void String::Allocate( const int length, bool after )
   {
   if ( length < 1 )
      return;

   int newSize = size + length;
   char* newCString = new char[ newSize + 1 ];

   if ( cstring != nullptr )
      memcpy( newCString, cstring, size );

   delete cstring;
   cstring = newCString;
   size = newSize;
   }


void String::RemoveWhitespace() 
  {
  if ( cstring == nullptr )
     return;

  char* i = cstring;
  char* j = cstring;
  while(*j != 0)
  {
    *i = *j++;
    if(!isspace(*i))
      i++;
  }
  *i = 0;
  }
