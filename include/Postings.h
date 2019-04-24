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
      FixedLengthString();
   private:
      static const unsigned MaxLength = 19;
      char characters[ MaxLength + 1 ];

   };

class FixedLengthURL
   {
   public:
      FixedLengthURL( const char* cstring );

      bool operator== ( const FixedLengthURL& other ) const;
      FixedLengthURL();
   private:
      static const unsigned MaxLength = 199;
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

namespace hash {
   template <> struct Hash<SubBlockInfo> {
   static uint64_t get(const SubBlockInfo& subBlockInfo) {
      return subBlockInfo.blockIndex + ((uint64_t)(subBlockInfo.subBlockIndex) << 32);
   }
   uint64_t operator()(const SubBlockInfo& subBlockInfo)
   {
      return get(subBlockInfo);
   }
   }; // this should help?
}


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
      SubBlock GetSubBlock( SubBlockInfo subBlockInfo, bool endWanted, bool writeLockHeld );
      void DeleteSubBlock( SubBlock subBlock );

      SubBlock GetOpenSubBlock( unsigned subBlockSize );
      SubBlock GetLastUsedSubBlock( unsigned subBlockSize, unsigned blockIndexHeld );

      void IncrementOpenSubBlock( unsigned subBlockSize, bool metaDataHeld );
      void IncrementNumBlocks( bool metaDataHeld );

      void SetLastUsed( SubBlockInfo lastUsed, bool metaDataHeld );
      void SetOpen( SubBlockInfo open, bool metaDataHeld );

      SubBlock MmapSubBlock( SubBlockInfo subBlockInfo, bool writing, bool writeLockHeld );
      void MunmapSubBlock( SubBlock subBlock );

      unsigned SmallestSubBlockSize( );

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
      threading::Mutex lockMapLock;
      threading::Mutex metaDataLock;
      threading::Mutex subBlockIndexLock;
      threading::Mutex wordIndexLock;

   friend SubBlock;
   friend IsrWord;
   };

#endif
