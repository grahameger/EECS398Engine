#ifndef POSTINGLIST_H
#define POSTINGLIST_H

#include "StringView.h"
#include "ByteStream.h"

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

   private:
      // To hold the existing data
      StringView posts;
      StringView index;
      // To hold the new data
      OutputByteStream newPosts;
      OutputByteStream newIndices;

      unsigned long long largestPosting;

      static const unsigned LowEndBits;

   };

#endif
