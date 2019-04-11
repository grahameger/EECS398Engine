#include <cstring>
#include "PostingList.h"
#include "Utf8Uint.h"

// How many bits we get rid of for our index offset
const unsigned PostingList::LowEndBits = 16;


PostingList::PostingList( ) : newIndices( false ), largestPosting( 0 )
   { }


PostingList::PostingList( StringView postingListData ) : newIndices( false )
   {
   unsigned indexLength = postingListData.GetInString< unsigned >( );
   unsigned postsLength = 
         postingListData.GetInString< unsigned >( sizeof( unsigned ) );
   // What our largest posting is for this number
   largestPosting =
         postingListData.GetInString< unsigned long long >( 2 * sizeof( unsigned ) );

   // CString + space for indexLength, postsLength, startLastIndex, and largestPosting
   char* postsStart = postingListData.RawCString( ) + 2 * sizeof( unsigned ) +
         sizeof( unsigned long long );
   // CString + Size - indexLength
   char* indexStart = postingListData.RawCString( ) + 
         postingListData.Size( ) - indexLength;

   posts = StringView( postsStart, postsLength );
   index = StringView( indexStart, indexLength, false );
   }


void PostingList::AddPosting( unsigned long long posting )
   {
   // Check if we need to add a new Index entry
   // Note that the OR is for adding when index is empty
   if ( posting >> LowEndBits > largestPosting >> LowEndBits || 
         ( index.Empty( ) && newIndices.Size( ) == 0 ) )
      {
      // Add posting
      newIndices << Utf8Uint( posting );
      // Add offset in posts
      newIndices << Utf8Uint( posts.Size( ) + newPosts.Size( ) );
      }

   // Add this delta to the newPosts
   newPosts << Utf8Uint( posting - largestPosting );

   // Update largestPosting
   largestPosting = posting;
   }


unsigned int PostingList::GetByteSize( )
   {
   return 2 * sizeof( unsigned ) + sizeof( unsigned long long ) + 
         posts.Size( ) + index.Size( ) + newPosts.Size( ) + newIndices.Size( );
   }


StringView PostingList::GetData( unsigned int postingListSize )
   {
   char* dataString = new char[ postingListSize ];
   StringView returnView( dataString, postingListSize );

   // Add indexSize and postsSize
   returnView.SetInString< unsigned >( index.Size( ) + newIndices.Size( ) );
   returnView.SetInString< unsigned >
         ( posts.Size( ) + newPosts.Size( ), sizeof( unsigned ) );
   // Add largestPosting
   returnView.SetInString< unsigned long long >
         ( largestPosting, 2 * sizeof( unsigned ) );

   char* postsStart = dataString + 
         2 * sizeof( unsigned ) + sizeof( unsigned long long );
   char* indexStart = dataString + postingListSize - 
         index.Size( ) - newIndices.Size( );

   // Add posts data
   memcpy( postsStart, posts.CString( ), posts.Size( ) );
   // Add index data
   memcpy( indexStart + newIndices.Size( ), index.CString( ), index.Size( ) );

   // Add newPosts data
   memcpy( postsStart + posts.Size( ), newPosts.GetString( ).CString( ), 
         newPosts.Size( ) );
   // Add newIndices data
   memcpy( indexStart, newIndices.GetString( ).CString( ), newIndices.Size( ) );

   return returnView;
   }


void PostingList::UpdateInPlace( StringView& toUpdate )
   {
   // Add indexSize and postsSize
   toUpdate.SetInString< unsigned >( index.Size( ) + newIndices.Size( ) );
   toUpdate.SetInString< unsigned >
         ( posts.Size( ) + newPosts.Size( ), sizeof( unsigned ) );
   // Add largestPosting
   toUpdate.SetInString< unsigned long long >
         ( largestPosting, 2 * sizeof( unsigned ) );

   char* postsStart = toUpdate.RawCString( ) + 
         2 * sizeof( unsigned ) + sizeof( unsigned long long );
   char* indexStart = toUpdate.RawCString( ) + toUpdate.Size( ) - 
         index.Size( ) - newIndices.Size( );

   // Add newPosts data
   memcpy( postsStart + posts.Size( ), newPosts.GetString( ).CString( ), 
         newPosts.Size( ) );
   // Add newIndices data
   memcpy( indexStart, newIndices.GetString( ).CString( ), newIndices.Size( ) );
   }
