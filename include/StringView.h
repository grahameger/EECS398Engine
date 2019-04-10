#ifndef STRINGVIEW_H
#define STRINGVIEW_H

// Note that StringView does NOT have ownership of it's CString and thus does not
// delete it when finished.
//
class StringView
   {
   public:
      StringView( const char* CString, int length, bool forwards = true );

      bool Empty( ) const;
      int Size( ) const;
      const char* GetCString( ) const;
      bool Compare( const StringView& other ) const;

      const char operator[ ] ( int index ) const;
      
      operator bool( ) const;

   private:
      const char* CString;
      int length;
      bool forwards;

   };

#endif
