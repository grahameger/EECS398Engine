#ifndef STRINGVIEW_H
#define STRINGVIEW_H

// Note that StringView does NOT have ownership of it's CString and thus does not
// delete it when finished.
//
class StringView
   {
   public:
      StringView( );
      StringView( char* CString, unsigned length, bool forwards = true );

      bool Empty( ) const;
      unsigned Size( ) const;
      const char* const CString( ) const;
      char* const RawCString( ) const;
      bool Compare( const StringView& other ) const;

      template < typename T >
      const T GetInString( unsigned offset = 0 ) const;
      template < typename T >
      void SetInString( T value, unsigned offset = 0 );

      const char operator[ ] ( int index ) const;
      
      operator bool( ) const;

   private:
      char* cString;
      unsigned length;
      bool forwards;

   };

#endif
