#ifndef POSTINGS_H
#define POSTINGS_H

#include <unordered_map>

#include "StringView.h"
#include "PersistentHashMap.h"
#include "threading.h"

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


// SubBlock

struct SubBlock
   {
   bool uninitialized;
   char* mmappedArea;
   unsigned dataOffset;
   SubBlockInfo subBlockInfo;
   threading::ReadWriteLock* rwlock;
   bool writingLock;

   void SetNextPtr( unsigned blockIndex );
   bool operator!= ( const SubBlock& other ) const;
   bool operator== ( const SubBlock& other ) const;
   StringView ToStringView( );
   };


struct IsrInfo
   {
   unsigned nextPtr;
   PostingList* postingList;
   SubBlock subBlock;
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

      IsrInfo GetPostingList( const FixedLengthString& word );
      IsrInfo GetPostingList( unsigned blockIndex );

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

      SubBlock MmapSubBlock( SubBlockInfo subBlockInfo, bool writing );
      void MunmapSubBlock( SubBlock subBlock );

      unsigned SmallestSubBlockSize( ) const;

      // Data
      StringView metaData;
      unsigned nextBlockIndex;
      int indexFD;

      // Word to subBlock
      PersistentHashMap< FixedLengthString, SubBlockInfo > subBlockIndex;
      // subBlock to word
      PersistentHashMap< SubBlockInfo, FixedLengthString > wordIndex;
      // subBlock to rwLock
      std::unordered_map< unsigned, threading::ReadWriteLock* > lockMap;

   friend SubBlock;
   friend IsrWord;
   };

#endif
