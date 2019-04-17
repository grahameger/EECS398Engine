#ifndef INDEX_H
#define INDEX_H

#include "StringView.h"
#include "PersistentHashMap.h"

class ParsedDocument;
class PostingListSubBlock;

template < typename T >
class Vector;


// FixedLengthString class
class FixedLengthString
   {
   public:
      FixedLengthString( const char* cstring );

      bool operator== ( const FixedLengthString& other ) const;

   private:
      static const unsigned MaxLength = 19;
      char characters[ MaxLength + 1 ];

   };


// SubBlockInfo class
struct SubBlockInfo
   {
   unsigned subBlockSize;
   unsigned blockIndex;
   unsigned char subBlockIndex;
   };


// Index class
class Index 
   {
   public:
      Index( const char* filename, unsigned blockSize = DefaultBlockSize,
            unsigned numSizes = DefaultNumSizes );
      void AddPostings( const FixedLengthString& word, 
            const Vector< unsigned long long >* postings );

   private:
      // Methods
      void CreateNewIndexFile( const char* filename, unsigned blockSize, 
            unsigned numSizes );

      PostingListSubBlock GetPostingList
            ( const FixedLengthString& word, bool endWanted = false );
      PostingListSubBlock GetNewPostingListSubBlock( const FixedLengthString& word );
      PostingListSubBlock GetPostingListSubBlock
            ( SubBlockInfo subBlockInfo, bool endWanted = false );

      SubBlockInfo GetOpenSubBlock( unsigned subBlockSize ) const;
      void IncrementOpenSubBlock( unsigned subBlockSize );

      char* MmapSubBlock( SubBlockInfo subBlockInfo );
      void MunmapSubBlock( SubBlockInfo subBlockInfo, char* subBlock );

      unsigned SmallestSubBlockSize( ) const;

      // Data
      StringView metaData;
      unsigned nextBlockIndex;
      unsigned blockSize;
      int indexFD;

      PersistentHashMap< FixedLengthString, SubBlockInfo > subBlockIndex;

      static const unsigned DefaultBlockSize;
      static const unsigned DefaultNumSizes;

   };

#endif
