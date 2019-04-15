#ifndef INDEXSCHEDULER_H
#define INDEXSCHEDULER_H

#include "PersistentHashMap.h"
#include "String.h"
#include "StringView.h"

class SchedulerSubBlock
   {
   public:
      SchedulerSubBlock( Pair< unsigned, unsigned char > subBlockLoc );
      SchedulerSubBlock( unsigned blockIndex, unsigned char subBlockIndex );

      unsigned GetBlockIndex( ) const;
      unsigned char GetSubBlockIndex( ) const;

   private:
      unsigned blockIndex;
      unsigned char subBlockIndex;
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

      SchedulerSubBlock GetPostingList( const String& word );

   private:
      IndexScheduler( unsigned blockSize, unsigned pagesSize, unsigned numSizes );
      void CreateNewIndexFile( unsigned blockSize, unsigned numSizes );

      Pair< unsigned, unsigned char > GetOpenSubBlock( unsigned subBlockSize );

      void IncrementOpenSubBlock( unsigned subBlockSize );
      void IncrementNumBlocks( );

      StringView metaData;
      char** pages;
      int indexFD;
      unsigned nextBlockIndex, blockSize;

      PersistentHashMap< const char*, Pair< unsigned, unsigned char > > subBlockIndex;

   };

#endif
