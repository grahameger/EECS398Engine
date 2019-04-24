// Created by Jason Setting
// Graham Eger added additional string concatenation functions on 4/2

#pragma once
#ifndef STRING_H_398
#define STRING_H_398

#include <string> // only for a copy constructor

class String
{
public:
   String( const unsigned length = 0 );
   String( const char single_char );
   String( const char* toCopy, unsigned length = 0 );
   String( char*&& toMove, unsigned length = 0 );
   String( const String& toCopy );
   String( const std::string& toCopy);
   String( String&& toMove );
   String& operator=( const String& toCopy );
   String& operator=( String&& toMove );
   ~String();
   void RemoveWhitespace( );
   void Swap( String& toSwap );
   void Allocate( const unsigned length, bool after = true );
   bool Empty( ) const;
   unsigned Size( ) const;
   const char* CString( ) const;
   bool Compare( const String& other ) const;
   const char operator[ ] ( unsigned index ) const;
   char& operator[ ] ( unsigned index );
   String& operator+= ( const String& rhs );
   String& operator+= ( const char );
   friend String operator+ ( const String lhs, const String& rhs );
   friend String operator+ ( const String lhs, const char * toCat );
   operator bool( ) const;
private:
    const static char* nullString;
    char* cstring;
    unsigned size;
};

#endif
