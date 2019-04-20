#include "ISR.h"
#include "Postings.h"
#include "PostingList.h"


IsrWord::IsrWord( String word )
      : validISR( true ), currentLocation( 0 )
   {
   auto returnPair = Postings::GetPostings( )->GetPostingList( word.CString( ) );
   nextPtr = returnPair.first;

   if ( returnPair.second )
      postingLists.push_back( returnPair.second );
   else
      validISR = false;
   }


Location IsrWord::NextInstance( )
   {
   return SeekToLocation( 0 );
   }


Location IsrWord::SeekToLocation( Location seekDestination )
   {
   if ( !validISR || postingLists.empty( ) )
      return currentLocation = 0;

   for ( unsigned i = 0; i < postingLists.size( ); i++ )
      {
      Location posting = postingLists[ i ]->GetPosting( seekDestination );
      if ( posting != 0 )
         return currentLocation = posting;
      }

   while ( nextPtr != 0 )
      {
      auto returnPair = Postings::GetPostings( )->GetPostingList( nextPtr );
      nextPtr = returnPair.first;
      postingLists.push_back( returnPair.second );

      Location posting = returnPair.second->GetPosting( seekDestination );
      if ( posting != 0 )
         return currentLocation = posting;
      }

   return currentLocation = 0;
   }


Location IsrWord::CurInstance( ) const
   {
   return currentLocation;
   }


IsrWord::operator bool( ) const
   {
   return validISR;
   }


IsrEndDoc::IsrEndDoc( ) : IsrWord( "" )
   { }


