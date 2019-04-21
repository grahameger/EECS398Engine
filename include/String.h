// Created by Jason Setting
// Graham Eger added additional string concatenation functions on 4/2

#pragma once
#ifndef STRING_H_398
#define STRING_H_398

#include <string> // only for a copy constructor

class String
{
public:
   String( const int length = 0 );
   String( const char single_char );
   String( const char* toCopy, int length = -1 );
   String( char*&& toMove, int length = -1 );
   String( const String& toCopy );
   String( const std::string& toCopy);
   String( String&& toMove );
   String& operator=( const String& toCopy );
   String& operator=( String&& toMove );
   ~String();
   void RemoveWhitespace( );
   void Swap( String& toSwap );
   void Allocate( const int length, bool after = true );
   bool Empty( ) const;
   int Size( ) const;
   const char* CString( ) const;
   bool Compare( const String& other ) const;
   const char operator[ ] ( int index ) const;
   char& operator[ ] ( int index );
   String& operator+= ( const String& rhs );
   String& operator+= ( const char );
   friend String operator+ ( const String lhs, const String& rhs );
   friend String operator+ ( const String lhs, const char * toCat );
   operator bool( ) const;
private:
    const static char* nullString;
    char* cstring;
    int size;
};

#endif
