#include "ISR.h"
#include "Postings.h"
#include "PostingList.h"


IsrWord::IsrWord( String word )
      : validISR( true ), currentLocation( 0 )
   {
   IsrInfo isrInfo = Postings::GetPostings( )->GetPostingList( word.CString( ) );
   nextPtr = isrInfo.nextPtr;

   if ( isrInfo.postingList )
      postingLists.push_back( isrInfo.postingList );
   else
      validISR = false;

   subBlocks.push_back( isrInfo.subBlock );
   }


IsrWord::~IsrWord( )
   {
   for( unsigned i = 0; i < postingLists.size( ); i++ )
      delete postingLists[ i ];
   for( unsigned i = 0; i < subBlocks.size( ); i++ )
      if ( subBlocks[ i ].mmappedArea != nullptr )
         Postings::GetPostings( )->MunmapSubBlock( subBlocks[ i ] );
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
      IsrInfo isrInfo = Postings::GetPostings( )->GetPostingList( nextPtr );
      nextPtr = isrInfo.nextPtr;
      postingLists.push_back( isrInfo.postingList );
      subBlocks.push_back( isrInfo.subBlock );

      Location posting = isrInfo.postingList->GetPosting( seekDestination );
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


IsrEndDoc::IsrEndDoc( ) : IsrWord( ( char* )"" )
   { }


