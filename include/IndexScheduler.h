#ifndef INDEXSCHEDULER_H
#define INDEXSCHEDULER_H

#include "StringView.h"

class SchedulerBlock;

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
      unsigned NumBlocks( );
      SchedulerBlock GetPostingList( );

   private:
      IndexScheduler( unsigned blockSize, unsigned pagesSize, unsigned numSizes );
      void CreateNewIndexFile( unsigned blockSize, unsigned numSizes );
      void IncrementOpenSubBlock( unsigned subBlockSize );
      void IncrementNumBlocks( );

      StringView metaData;
      char** pages;
      int indexFD;
      unsigned nextBlockIndex, blockSize;

   };

#endif
