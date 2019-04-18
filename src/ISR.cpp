#include "ISR.h"
#include "Index.h"
#include "PostingList.h"


ISR::ISR( String& word ) : children( nullptr )
   { }


ISR::ISR( ) : children( nullptr )
   { }


unsigned long long ISR::NextInstance( unsigned long long after )
   {
   return 0;
   }


WordISR::WordISR( String& word )
      : postingList( Index::GetIndex( )->GetPostingList( word.CString( ) ) )
   { }


unsigned long long WordISR::NextInstance( unsigned long long after )
   {
   if ( !postingList )
      return 0;

   return postingList->GetPosting( after );
   }


