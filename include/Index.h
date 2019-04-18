#ifndef INDEX_H
#define INDEX_H

#include "StringView.h"
#include "PersistentHashMap.h"

class SubBlock;
class PostingList;
class WordISR;

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

   bool operator!= ( const SubBlockInfo& other )
      { 
      return blockIndex == other.blockIndex && 
            subBlockIndex == other.subBlockIndex; 
      }
   };


// Index class
class Index 
   {
   // Static
   public:
      static Index* GetIndex( );

   private:
      static Index* CurIndex;
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
      Index( );

      void CreateNewIndexFile( );
      void SaveSplitPostingList( SubBlock plSubBlock, StringView plStringView, 
            Vector< PostingList* >& split, const FixedLengthString& word );

      PostingList* GetPostingList( const FixedLengthString& word );

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

   friend SubBlock;
   friend WordISR;
   };

#endif
