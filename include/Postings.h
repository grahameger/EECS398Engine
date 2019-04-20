#ifndef POSTINGS_H
#define POSTINGS_H

#include "StringView.h"
#include "PersistentHashMap.h"

class SubBlock;
class PostingList;
class IsrWord;

template < typename T >
class Vector;


// FixedLengthString class
class FixedLengthString
   {
   public:
      FixedLengthString( const char* cstring );

      bool operator== ( const FixedLengthString& other ) const;
      FixedLengthString();
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

   bool operator!= ( const SubBlockInfo& other )
      { 
      return blockIndex != other.blockIndex || 
            subBlockIndex != other.subBlockIndex; 
      }

   bool operator== ( const SubBlockInfo& other )
      {
      return blockIndex == other.blockIndex &&
            subBlockIndex == other.subBlockIndex;
      }
   };


// Postings class
class Postings 
   {
   // Static
   public:
      static Postings* GetPostings( );

   private:
      static Postings* CurPostings;
      static unsigned blockSize;
      static const unsigned BlockDataOffset;
      static const unsigned DefaultBlockSize;
      static const unsigned DefaultNumSizes;
      static const char* Filename;

   // Instance
   public:
      void AddPostings( const FixedLengthString& word, 
            const Vector< unsigned long long >* postings );

   private:
      // Methods
      Postings( );

      void CreateNewPostingsFile( );
      void SaveSplitPostingList( SubBlock plSubBlock, StringView plStringView, 
            Vector< PostingList* >& split, const FixedLengthString& word );

      Pair< unsigned, PostingList* > GetPostingList
            ( const FixedLengthString& word );
      Pair< unsigned, PostingList* > GetPostingList
            ( unsigned blockIndex );

      SubBlock GetPostingListSubBlock
            ( const FixedLengthString& word, bool endWanted = false );

      SubBlock GetNewSubBlock( unsigned minSize );
      SubBlock GetSubBlock( SubBlockInfo subBlockInfo, bool endWanted = false );
      void DeleteSubBlock( SubBlock subBlock );

      SubBlockInfo GetOpenSubBlock( unsigned subBlockSize ) const;
      SubBlockInfo GetLastUsedSubBlock( unsigned subBlockSize ) const;

      void IncrementOpenSubBlock( unsigned subBlockSize );
      void IncrementNumBlocks( );

      void SetLastUsed( SubBlockInfo lastUsed );
      void SetOpen( SubBlockInfo open );

      SubBlock MmapSubBlock( SubBlockInfo subBlockInfo );
      void MunmapSubBlock( SubBlock subBlock );

      unsigned SmallestSubBlockSize( ) const;

      // Data
      StringView metaData;
      unsigned nextBlockIndex;
      int indexFD;

      PersistentHashMap< FixedLengthString, SubBlockInfo > subBlockIndex;
      PersistentHashMap< SubBlockInfo, FixedLengthString > wordIndex;

   friend SubBlock;
   friend IsrWord;
   };

#endif
