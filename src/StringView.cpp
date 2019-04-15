#include "StringView.h"

StringView::StringView( ) : cString( nullptr ), length( 0 ), forwards( true )
   { }

StringView::StringView( char* cString, unsigned length, bool forwards )
      : cString( cString ), length( length ), forwards( forwards )
   { }


bool StringView::Empty( ) const
   {
   return length == 0;
   }


unsigned StringView::Size( ) const
   {
   return length;
   }


const char* const StringView::CString( ) const
   {
   return cString;
   }


char* const StringView::RawCString( ) const
   {
   return cString;
   }


template <typename T>
const T StringView::GetInString( unsigned offset ) const
   {
   if ( forwards )
      return *( T* )( cString + offset );
   
   unsigned char forwardString[ sizeof( T ) ];
   for ( unsigned i = 0; i < sizeof( T ); i++ )
      forwardString[ i ] = operator[ ]( i + offset );

   return *( T* )( forwardString );
   }

// Here are the ones I use
template const unsigned char
      StringView::GetInString< unsigned char >( unsigned offset ) const;
template const unsigned 
      StringView::GetInString< unsigned >( unsigned offset ) const;
template const unsigned long long 
      StringView::GetInString< unsigned long long >( unsigned offset ) const;


template <typename T>
void StringView::SetInString( T value, unsigned offset )
   {
   if ( forwards )
      {
      *( T* )( cString + offset ) = value;
      return;
      }
   
   for ( unsigned i = 0; i < sizeof( T ); i++ )
      *( T* )( cString + length - offset - i ) = *( ( char* )&value + i );
   }

// Here are the ones I use
template void StringView::SetInString< unsigned char >
      ( unsigned char value, unsigned offset );
template void StringView::SetInString< unsigned >
      ( unsigned value, unsigned offset );
template void StringView::SetInString< unsigned long long >
      ( unsigned long long value, unsigned offset );


const char StringView::operator[ ] ( int index ) const
   {
   int newIndex = forwards ? index : ( length - index - 1 );
   return cString[ newIndex ];
   }


bool StringView::Compare( const StringView& other ) const
   {
   if ( length != other.length )
      return false;

   for ( unsigned i = 0; i < length; i++ )
      {
      unsigned newIndex = forwards ? i : ( length - i );
      if ( cString[ newIndex ] != other[ i ] )
         return false;
      }

   return true;
   }


StringView::operator bool( ) const
   {
   return length > 0;
   }


