#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "IndexScheduler.h"

// 128 pages of 16kb each is 2Gb total
const unsigned IndexScheduler::DefaultBlockSize = 16'384;
const unsigned IndexScheduler::DefaultPagesSize = 128;
// 9 sizes on default blocks of 16384 gives us sizes
// 16384, 8192, 4096, 2048, 1024, 512, 256, 128, 64
const unsigned IndexScheduler::DefaultNumSizes = 9;

IndexScheduler* IndexScheduler::Scheduler = nullptr;
char* IndexScheduler::IndexFilename = nullptr;

// Static public methods

IndexScheduler* IndexScheduler::GetScheduler
      ( unsigned blockSize, unsigned pagesSize, unsigned numSizes )
   {
   if ( !Scheduler )
      Scheduler = new IndexScheduler( blockSize, pagesSize, numSizes );

   return Scheduler;
   }


// Instance public methods

unsigned IndexScheduler::NumBlocks( )
   {
   return metaData.GetInString< unsigned >( sizeof( unsigned ) );
   }


// Instance private methods

IndexScheduler::IndexScheduler
      ( unsigned blockSize, unsigned pagesSize, unsigned numSizes )
      : pages( new char* [ pagesSize ] ), 
      indexFD( open( IndexFilename, O_RDWR ) ), blockSize( blockSize )
   {
   if ( indexFD == -1 )
      CreateNewIndexFile( blockSize, numSizes );
   else
      {
      char* map = ( char* )mmap( 
            nullptr, blockSize, PROT_READ | PROT_WRITE, MAP_SHARED, indexFD, 0 );
      metaData = StringView( map, blockSize );
      }
   }


void IndexScheduler::CreateNewIndexFile( unsigned blockSize, unsigned numSizes )
   {
   indexFD = open( IndexFilename, O_RDWR | O_CREAT, S_IRWXU );
   
   if ( indexFD == -1 )
      {
      fprintf( stderr, "Error creating file %s, %s\n", IndexFilename, 
            strerror( errno ) );
      exit( 1 );
      }

   ftruncate( indexFD, blockSize * 2 );

   char* map = ( char* )mmap( 
         nullptr, blockSize, PROT_READ | PROT_WRITE, MAP_SHARED, indexFD, 0 );

   metaData = StringView( map, blockSize );
   unsigned currentOffset = 0;

   // Set blockSize in metadata
   metaData.SetInString< unsigned >( blockSize );
   currentOffset += sizeof( unsigned );
   // Set numBlocks in metadata
   metaData.SetInString< unsigned >( numSizes + 1, currentOffset );
   currentOffset += sizeof( unsigned );
   // Set numSizes info in metadata
   metaData.SetInString< unsigned >( numSizes, currentOffset );

   nextBlockIndex = 1;

   // Set open Blocks/subBlocks for sizes in metadata
   for ( unsigned curSize = blockSize; curSize > ( blockSize >> numSizes ); 
         curSize >>= 1 )
      {
      IncrementOpenSubBlock( curSize );
      }
   }


void IndexScheduler::IncrementOpenSubBlock( unsigned subBlockSize )
   {
   // Offset for the open location of this subBlock info
   unsigned offset = sizeof( unsigned ) +
         ( blockSize / subBlockSize * sizeof( unsigned ) * 2 );

   unsigned oldBlockIndex = metaData.GetInString< unsigned >( offset );

   // Old blockIndex was set to 0 (aka not set)
   if ( !oldBlockIndex )
      {
      metaData.SetInString< unsigned >( nextBlockIndex++, offset );
      return;
      }
      
   unsigned oldSubBlock = metaData.GetInString< unsigned >
         ( offset + sizeof( unsigned ) );

   // Need a new block
   if ( ++oldSubBlock == blockSize / subBlockSize )
      {
      // TODO
      }
   else
      metaData.SetInString< unsigned >( oldSubBlock, offset + sizeof( unsigned ) );
   }


void IndexScheduler::IncrementNumBlocks( )
   {
   unsigned oldNum = NumBlocks( );
   metaData.SetInString< unsigned >( oldNum + 1, sizeof( unsigned ) );
   }


