#ifndef POSTINGLIST_H
#define POSTINGLIST_H

#include "StringView.h"
#include "ByteStream.h"

template < typename T >
class Vector;

// This is a class to manage the functions of our posting lists. To start, you
//  can create a postingList either from scratch, or by passing it a StringView
//  of the data it's in.
//    For example, say we have a block with 16 posting lists in it, and we want
//    the 14th. We would pass the constructor a stringview of the memory mapped
//    block + 14 * ( BlockSize / 16 ) with a length of BlockSize / 16.
//  the constructor uses the information in this subBlock to populate our data
//  structure.
//
// Once we have a PostingList object, we can call AddPosting as many times as we
//  want on it, to add n postings to this PostingList. When it comes time to put
//  this PostingList back in memory, the owner of this PostingList in some higher
//  part of the code will call GetByteSize( ) to see how many bytes this
//  PostingList now will consume, and can then decide to either:
//    A) Modify the data in place because it does not need a larger subBlock
//       ( This is the UpdateInPlace function )
//    B) Get the PostingList as a String of size n because we need a larger
//     subBlock of size n now.
//       ( This is the GetData function )
// 
// The workings of the class are relativley simple. We hold a StringView for the
//  posts and index as they were when we read them in, and then OutputByteStreams
//  for the additions we will make to posts and index (because they only grow in
//  one direction). We keep one other local variable that is largestPosition
//  because we only write this to disk when GetData or UpdateInPlace is called.
//
// For Max:
//  There are some slight design tweaks from when we talked prior. First of all,
//  there are only three fixed width numbers in the block and they all appear at
//  the beginning in sequence. These numbers are: ( unsigned ) indexLength,
//  ( unsigned ) postsLength, and ( unsigned long long ) largestPosting, in order.
//  Note that we don't need a pointer to the offset for the next post because it
//  can be calculated from posts.Size( ) and newPosts.Size( ) (see the GetData
//  function). Note that we also don't need to read the last index entry either
//  anymore. We just compare our higher-order bits to those of largestPosition
//  now and assume that if they're the same, an indexEntry has already been made
//  (see the AddPosting function).
//
class PostingList
   {
   public:
      PostingList( );
      // postingListData is a stringView of part of an mmapped block
      PostingList( StringView postingListData );

      // Add a posting to this posting list
      void AddPosting( unsigned long long posting );

      // Get the byte size this will be if printed
      unsigned int GetByteSize( );
      // Get the data placed into a string of a certain size
      StringView GetData( unsigned int postingListSize );
      // Update the existing postingListData with the new info in place
      void UpdateInPlace( StringView& postingListData );

      // Split the posting list into n posting lists of maximum size BlockSize
      Vector< PostingList* > Split( unsigned blockSize );

   private:
      // Used in Split
      PostingList( StringView posts, StringView index, unsigned long long prevLargest );

      // To hold the existing data
      StringView posts;
      StringView index;
      // To hold the new data
      OutputByteStream newPosts;
      OutputByteStream newIndices;

      unsigned long long largestPosting;
      unsigned long long origLargestPosting;

      static const unsigned LowEndBits;

   };

#endif
