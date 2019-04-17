#ifndef INDEXSCHEDULER_H
#define INDEXSCHEDULER_H

#include "PersistentHashMap.h"
#include "String.h"
#include "StringView.h"

class FixedLengthString
   {
   public:
      FixedLengthString( const char* cstring );

      bool operator== ( const FixedLengthString& other ) const;

   private:
      static const unsigned MaxLength = 19;
      char characters[ MaxLength + 1 ];

   };


using SubBlockLoc = Pair< unsigned, unsigned char >;
using SubBlockInfo = Pair< unsigned, SubBlockLoc >;

class SchedulerSubBlock
   {
   public:
      SchedulerSubBlock( SubBlockInfo subBlockInfo, bool initialized = true );
      ~SchedulerSubBlock( );

      bool GetInitialized( ) const;
      unsigned GetBlockSize( ) const;
      unsigned GetBlockIndex( ) const;
      unsigned char GetSubBlockIndex( ) const;
      StringView GetStringView( );

   private:
      unsigned blockIndex;
      unsigned char subBlockIndex;
      unsigned blockSize;
      bool initialized;
      bool inMemory;

   };


class IndexScheduler
   {
   // Static
   public:
      static IndexScheduler* GetScheduler( unsigned blockSize = DefaultBlockSize,
            unsigned pagesSize = DefaultPagesSize, unsigned numSizes = DefaultNumSizes );

      static char* IndexFilename;

   private:
      static IndexScheduler* Scheduler;
      static const unsigned DefaultBlockSize;
      static const unsigned DefaultPagesSize;
      static const unsigned DefaultNumSizes;
   
   // Instance
   public:
      unsigned NumBlocks( ) const;
      unsigned char SmallestSubBlockSize( ) const;

      SchedulerSubBlock GetPostingList( const FixedLengthString& word );

   private:
      IndexScheduler( unsigned blockSize, unsigned pagesSize, unsigned numSizes );
      void CreateNewIndexFile( unsigned blockSize, unsigned numSizes );

      SubBlockInfo GetOpenSubBlock( unsigned subBlockSize );
      StringView GetSubBlockString( SubBlockLoc subBlockLoc );

      void DecrementSubBlockReferences( SubBlockLoc subBlockLoc );
      void IncrementSubBlockReferences( SubBlockLoc subBlockLoc );
      void IncrementOpenSubBlock( unsigned subBlockSize );
      void IncrementNumBlocks( );

      StringView metaData;
      char** pages;
      int indexFD;
      unsigned nextBlockIndex, blockSize;

      PersistentHashMap< FixedLengthString, SubBlockInfo > subBlockIndex;

   friend SchedulerSubBlock;
   };

#endif
