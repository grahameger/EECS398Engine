#include <cstring>
#include "PostingList.h"
#include "Utf8Uint.h"
#include "vector.h"

// How many bits we get rid of for our index offset
const unsigned PostingList::LowEndBits = 16;


PostingList::PostingList( ) : newIndices( false ), largestPosting( 0 ),
      origLargestPosting( 0 )
   { }


PostingList::PostingList( StringView posts, StringView index, 
      unsigned long long prevLargestPosting ) 
      : posts( posts ), index( index ), newIndices( false ),
      largestPosting( prevLargestPosting ), origLargestPosting( prevLargestPosting )
   { }


PostingList::PostingList( StringView postingListData ) : newIndices( false )
   {
   unsigned indexLength = postingListData.GetInString< unsigned >( );
   unsigned postsLength = 
         postingListData.GetInString< unsigned >( sizeof( unsigned ) );
   // What our largest posting is for this number
   largestPosting =
         postingListData.GetInString< unsigned long long >( 2 * sizeof( unsigned ) );
   origLargestPosting = largestPosting;

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


Vector< PostingList* > PostingList::Split( unsigned blockSize )
   {
   Vector< PostingList* > returnList( 1 );

   // It fits in one block
   if ( GetByteSize( ) <= blockSize )
      {
      returnList.push_back( this );
      return returnList;
      }

   // It doesn't fit in one block
   PostingList* curList = new PostingList( posts, index, origLargestPosting );
   // Turn OBS into IBS (we're reading not appending now)
   InputByteStream posts( newPosts.GetString( ) );
   InputByteStream index( newIndices.GetString( ) );

   unsigned offsetSubtractor = 0;
   bool firstEntry = true;
   unsigned long long curLargestPosting = origLargestPosting;

   // While we haven't caught up with data added
   while ( curLargestPosting != largestPosting )
      {
      // TODO: Better way to get Utf8Uint sizes
      OutputByteStream sizeChecker;
      Utf8Uint delta, posting, offset;
      bool hasIndex = false;
      
      posts >> delta;
      sizeChecker << delta;

      // If post has a corresponding index entry
      if ( curLargestPosting >> LowEndBits != 
            ( curLargestPosting + delta.GetValue( ) ) >> LowEndBits ||
            firstEntry )
         {
         hasIndex = true;
         firstEntry = false;
         index >> posting >> offset;
         offset = Utf8Uint( offset.GetValue( ) - offsetSubtractor );
         sizeChecker << posting << offset;
         }

      unsigned numBytes = sizeChecker.Size( );

      // If these bytes don't fit in current postingList
      if ( curList->GetByteSize( ) + numBytes > blockSize )
         {
         returnList.push_back( curList );
         curList = new PostingList( );
         offsetSubtractor += curList->posts.Size( ) + curList->newPosts.Size( );

         if ( !hasIndex )
            {
            hasIndex = true;
            posting = Utf8Uint( curLargestPosting + delta.GetValue( ) );
            offset = Utf8Uint( 0 );
            sizeChecker << posting << offset;
            numBytes = sizeChecker.Size( );
            }
         }

      curList->newPosts << delta;

      if ( hasIndex )
         curList->newIndices << posting << offset;

      curList->largestPosting = curLargestPosting = 
            curLargestPosting + delta.GetValue( );

      }

   returnList.push_back( curList );

   return returnList;
   }


