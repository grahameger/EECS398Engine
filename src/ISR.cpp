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
   { 
   auto returnPair = Index::GetIndex( )->GetPostingList( word.CString( ) );
   nextPtr = returnPair.first;
   postingLists.push_back( returnPair.second );
   }


unsigned long long WordISR::NextInstance( unsigned long long after )
   {
   if ( postingLists.empty( ) )
      return 0;

   for ( unsigned i = 0; i < postingLists.size( ); i++ )
      {
      unsigned long long posting = postingLists[ i ]->GetPosting( after );
      if ( posting != 0 )
         return posting;
      }

   while ( nextPtr != 0 )
      {
      auto returnPair = Index::GetIndex( )->GetPostingList( nextPtr );
      nextPtr = returnPair.first;
      postingLists.push_back( returnPair.second );

      unsigned long long posting = returnPair.second->GetPosting( after );
      if ( posting != 0 )
         return posting;
      }

   return 0;
   }


