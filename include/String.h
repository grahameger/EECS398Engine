// Created by Jason Setting
// Graham Eger added additional string concatenation functions on 4/2

#pragma once
#ifndef STRING_H_398
#define STRING_H_398

class String
   {
   public:
      String( const int length = 0 );
      String( const char single_char );
      String( const char* toCopy, int length = -1 );
      String( char*&& toMove, int length = -1 );
      String( const String& toCopy );
      String( String&& toMove );
      String& operator=( const String& toCopy );
      String& operator=( String&& toMove );
      ~String();

      bool Empty( ) const;
      int Size( ) const;
      const char* CString( ) const;

      void Allocate( const int size, bool after = true );
      void RemoveWhitespace( );
      void Swap( String& toSwap );
      bool Compare( const String& other ) const;

      const char operator[ ] ( int index ) const;
      char& operator[ ] ( int index );

      String& operator+= ( const String& rhs );
      friend String operator+ ( String lhs, const String& rhs );
      friend String operator+ ( String lhs, const char * toCat );

      operator bool( ) const;

   private:
      const static char* nullString;
      char* cstring;
      int size;

   };
