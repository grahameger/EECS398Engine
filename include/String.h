#pragma once
#ifndef STRING_H_398
#define STRING_H_398

class String
   {
public:
   String( );
   String( const char* toCopy );
   String( char*&& toMove );
   String( const String& toCopy );
   String( String&& toMove );
   String& operator=( const String& toCopy );
   String& operator=( String&& toMove );
   ~String();
   void RemoveWhitespace( );
   void Swap( String& toSwap );
   bool Empty( ) const;
   int Size( ) const;
   const char* CString( ) const;
   bool Compare( const String& other ) const;
   const char operator[ ] ( int index ) const;
   char& operator[ ] ( int index );

   String& operator+= ( const String& rhs );
   friend String operator+ ( String lhs, const String& rhs );

   operator bool( ) const;

private:
   const static char* nullString;
   char* cstring;
   int size;
   };

#endif
