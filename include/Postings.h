#ifndef POSTINGS_H
#define POSTINGS_H

#include <unordered_map>

#include "StringView.h"
#include "PersistentHashMap.h"
#include "threading.h"
#include "hash.h"

class PostingList;
class IsrWord;

template < typename T >
class Vector;


// FixedLengthString class
class FixedLengthString
   {
   public:
      FixedLengthString( const char* cstring );

      const char* Characters( ) const { return characters; }

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

      const char* Characters( ) const { return characters; }

      bool operator== ( const FixedLengthURL& other ) const;
      FixedLengthURL();
   private:
      static const unsigned MaxLength = 255;
      char characters[ MaxLength + 1 ];

   };
// SubBlockInfo class
struct SubBlockInfo
   {
   unsigned subBlockSize;
   unsigned blockIndex;
   unsigned char subBlockIndex;

   bool operator!= ( const SubBlockInfo& other ) const
      { 
      return blockIndex != other.blockIndex || 
            subBlockIndex != other.subBlockIndex; 
      }

   bool operator== ( const SubBlockInfo& other ) const
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
   unsigned AvailableSize( ) const
      { return subBlockInfo.subBlockSize - dataOffset; }
   };

namespace std 
   {
   template <> 
   struct hash< SubBlockInfo >
      {
      std::size_t operator( )( const SubBlockInfo &sbi ) const
         {
         return std::hash< unsigned >( )( sbi.blockIndex ) ^
               std::hash< unsigned >( )( sbi.subBlockSize ) ^
               std::hash< unsigned char >( )( sbi.subBlockIndex );
         }
      };
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
      static threading::Mutex constructorMutex;

   // Instance
   public:
      void AddPostings( const FixedLengthString& word, 
            const Vector< unsigned long long >* postings );

   private:
      // Methods
      Postings( );

      void CreateNewPostingsFile( );
      void SaveSplitPostingList( SubBlock plSubBlock,
            Vector< PostingList* >& split, const FixedLengthString& word );

      IsrInfo GetPostingList( const FixedLengthString& word );
      IsrInfo GetPostingList( unsigned blockIndex );

      SubBlock GetPostingListSubBlock
            ( const FixedLengthString& word, bool endWanted = false );

      // Necessary?
      SubBlock GetNewSubBlock( unsigned minSize );

      SubBlock GetOpenSubBlock( unsigned subBlockSize );
      SubBlock GetLastUsedSubBlock( unsigned subBlockSize, SubBlockInfo subBlockHeld );
      SubBlock GetSubBlock( SubBlockInfo subBlockInfo, bool endWanted, bool writeLockHeld );

      void DeleteSubBlock( SubBlock subBlock );

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
      threading::Mutex subBlockIndexLock;
      // subBlock to word
      PersistentHashMap< SubBlockInfo, FixedLengthString > wordIndex;
      threading::Mutex wordIndexLock;
      // subBlock to rwLock
      std::unordered_map< SubBlockInfo, threading::ReadWriteLock* > lockMap;
      threading::Mutex lockMapLock;

      threading::Mutex metaDataLock;
      threading::Mutex acquiringSubBlockLock;

   friend SubBlock;
   friend IsrWord;
   };

#endif
