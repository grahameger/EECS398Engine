// Created by Jason Setting
// Graham Eger added additional string concatenation functions on 4/2

#include <cstring>
#include <cctype>
#include "String.h"

const char* String::nullString = "";

String::String( const unsigned size ) : cstring( nullptr ), size( size )
   {
   if ( size > 0 )
      {
      cstring = new char[ size + 1 ];
      cstring[ size ] = 0;
      }
   }

String::String( const char single_char ) : size( 1 )
   {
   cstring = new char[ size + 1 ];
   cstring[0] = single_char;
   cstring[1] = 0;
   }

String::String( const char* toCopy, unsigned length ) : size( length )
   {
   if ( length == 0 )
      size = strlen( toCopy );
   if ( size == 0 )
      {
      cstring = nullptr;
      return;
      }
   cstring = new char[ size + 1 ];
   cstring[ size ] = 0;
   memcpy( cstring, toCopy, size );
   }


String::String( char*&& toMove, unsigned length ) : cstring( toMove ), 
      size( length )
   {
   toMove = nullptr;
   if ( length == 0 )
      size = strlen( cstring );
   }


String::String( const String& toCopy )
   {
   size = toCopy.size;

   if ( size == 0 || toCopy.cstring == nullptr )
      {
      cstring = nullptr;
	   return;
	   }
   
   cstring = new char[ size + 1 ];
   cstring[size] = 0;
   memcpy( cstring, toCopy.cstring, size );
   }


String::String( String&& toMove )
   {
   size = toMove.size;
   cstring = toMove.cstring;
   toMove.cstring = nullptr;
   }

String::String( const std::string& toCopy ) 
{
    size = toCopy.size();
    if ( size ) {
        cstring = new char [ size + 1 ];
        memcpy( cstring, toCopy.c_str( ), size + 1 );
    } else {
        cstring = nullptr;
    }
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
   unsigned tempSize = size;
   cstring = toSwap.cstring;
   size = toSwap.size;
   toSwap.cstring = tempCString;
   toSwap.size = tempSize;
   }


bool String::Empty( ) const { return size == 0; }


unsigned String::Size( ) const { return size; }


const char* String::CString( ) const
   {
   return cstring == nullptr ? nullString : cstring;
   }


bool String::Compare( const String& other ) const
   {
   if ( size != other.size ) return false;
    
   unsigned index = 0, i;
   while ( ( i = index++ ) != size && cstring[ i ] == other.cstring[ i ]) { }
    
   return index > size;
   }


const char String::operator[ ] ( unsigned index ) const
   {
   if ( cstring == nullptr )
      return nullString[ index ];
    
   return cstring[ index ];
   }


char& String::operator[ ] ( unsigned index )
   {
   if ( cstring == nullptr )
      {
      cstring = new char[ 1 ];
      cstring[ 0 ] = 0;
      return cstring[ index ];
      }

   return cstring[ index ];
   }


String& String::operator+= ( const String& rhs )
   {
   if ( rhs.cstring == nullptr )
      return *this;

   unsigned newSize = size + rhs.size;
   char* newCString = new char[ newSize + 1 ];

   if ( cstring != nullptr )
      memcpy( newCString, cstring, size );
   memcpy( newCString + size, rhs.cstring, rhs.size );
   newCString[ newSize ] = 0;

   if (cstring != nullString)
      delete[ ] cstring;
   cstring = newCString;
   size = newSize;

   return *this;
   }

   String& String::operator+= ( const char rhs )
      {
      unsigned newSize = size + 1;
      char* newCString = new char [ newSize + 1 ];

      if ( cstring != nullptr )
         memcpy( newCString, cstring, size );
      newCString[ size ] = rhs;
      newCString[ newSize ] = 0;

      if (cstring != nullString)
         delete[ ] cstring;
      cstring = newCString;
      size = newSize;

      return *this;
      }
 
 String operator+( const String lhs, const String& rhs) 
   {
   auto temp = lhs;
   temp += rhs;
   return temp;
   }

String::operator bool( ) const
   {
   return size > 0;
   }


void String::Allocate( const unsigned length, bool after )
   {
   if ( length < 1 )
      return;

   unsigned newSize = size + length;
   char* newCString = new char[ newSize + 1 ];

   if ( cstring != nullptr )
      memcpy( newCString, cstring, size );

   delete[ ] cstring;
   cstring = newCString;
   size = newSize;
   }


void String::RemoveWhitespace( ) 
   {
   if ( cstring == nullptr )
     return;

   char* i = cstring;
   char* j = cstring;
   while( *j != 0 )
      {
      *i = *j++;
      if( !isspace( *i ) )
      i++;
      }

   *i = 0;
   }


String operator+ ( const String lhs, const char * toCat ) 
   {
   auto cat = String( toCat );
   auto lhsString = lhs;
   lhsString += cat;
   return lhsString;
   }


