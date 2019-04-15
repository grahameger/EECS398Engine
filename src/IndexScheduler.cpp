#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "IndexScheduler.h"

// #####################
// Other Data Structures
// #####################

SchedulerSubBlock::SchedulerSubBlock( Pair< unsigned, unsigned char > subBlockLoc )
      : blockIndex( subBlockLoc.first ), subBlockIndex( subBlockLoc.second ),
      inMemory( false )
   { }


SchedulerSubBlock::SchedulerSubBlock
      ( unsigned blockIndex, unsigned char subBlockIndex )
      : blockIndex( blockIndex ), subBlockIndex( subBlockIndex ), inMemory( false )
   { }


unsigned SchedulerSubBlock::GetBlockIndex( ) const
   { return blockIndex; }
      

unsigned char SchedulerSubBlock::GetSubBlockIndex( ) const
   { return subBlockIndex; }


// ################
// Static Variables
// ################

// 128 pages of 16kb each is 2Gb total
const unsigned IndexScheduler::DefaultBlockSize = 16'384;
const unsigned IndexScheduler::DefaultPagesSize = 128;
// 9 sizes on default blocks of 16384 gives us sizes
// 16384, 8192, 4096, 2048, 1024, 512, 256, 128, 64
const unsigned IndexScheduler::DefaultNumSizes = 9;

// Initialy, there is no scheduler instance
IndexScheduler* IndexScheduler::Scheduler = nullptr;
// Initially, there is no filename
char* IndexScheduler::IndexFilename = nullptr;

// #####################
// Static public methods
// #####################

IndexScheduler* IndexScheduler::GetScheduler
      ( unsigned blockSize, unsigned pagesSize, unsigned numSizes )
   {
   if ( !Scheduler )
      Scheduler = new IndexScheduler( blockSize, pagesSize, numSizes );

   return Scheduler;
   }


// #######################
// Instance public methods
// #######################

unsigned IndexScheduler::NumBlocks( ) const
   {
   return metaData.GetInString< unsigned >( sizeof( unsigned ) );
   }


unsigned char IndexScheduler::SmallestSubBlockSize( ) const
   {
   unsigned numSizes = metaData.GetInString< unsigned >( 2 * sizeof( unsigned ) );
   return blockSize >> ( numSizes - 1 );
   }


SchedulerSubBlock IndexScheduler::GetPostingList( const String& word )
   {
   auto wordIt = subBlockIndex.find( word.CString( ) );

   // word not found
   if ( wordIt != subBlockIndex.end( ) )
      {
      }

   auto subBlockLoc = GetOpenSubBlock( SmallestSubBlockSize( ) );
   subBlockIndex.insert( Pair( word.CString( ), subBlockLoc ) );

   IncrementOpenSubBlock( SmallestSubBlockSize( ) );
   return SchedulerSubBlock( subBlockLoc );
   }


// ########################
// Instance private methods
// ########################

IndexScheduler::IndexScheduler
      ( unsigned blockSize, unsigned pagesSize, unsigned numSizes )
      : pages( new char* [ pagesSize ] ), 
      indexFD( open( IndexFilename, O_RDWR ) ), blockSize( blockSize ),
      subBlockIndex( String( IndexFilename ) + String( "_hash" ) )
   {
   // If file needs to be created
   if ( indexFD == -1 )
      CreateNewIndexFile( blockSize, numSizes );
   // Else read in member variables
   else
      {
      char* map = ( char* )mmap( 
            nullptr, blockSize, PROT_READ | PROT_WRITE, MAP_SHARED, indexFD, 0 );
      metaData = StringView( map, blockSize );

      blockSize = metaData.GetInString< unsigned >( );
      nextBlockIndex = NumBlocks( );
      }
   }


void IndexScheduler::CreateNewIndexFile( unsigned blockSize, unsigned numSizes )
   {
   indexFD = open( IndexFilename, O_RDWR | O_CREAT, S_IRWXU );
   
   // If file does not open
   if ( indexFD == -1 )
      {
      fprintf( stderr, "Error creating file %s, %s\n", IndexFilename, 
            strerror( errno ) );
      exit( 1 );
      }

   // Set the new file's size
   ftruncate( indexFD, blockSize * numSizes + 1 );

   // Mmap R/W the first block (metadata)
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


Pair< unsigned, unsigned char > IndexScheduler::GetOpenSubBlock
      ( unsigned subBlockSize )
   {
   unsigned offset = sizeof( unsigned ) +
         ( blockSize / subBlockSize * sizeof( unsigned ) * 2 );

   Pair< unsigned, unsigned char > returnPair;

   returnPair.first = metaData.GetInString< unsigned >( offset );
   offset += sizeof( unsigned );
   returnPair.second = metaData.GetInString< unsigned char >( offset );

   return returnPair;
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
      
   unsigned char oldSubBlock = metaData.GetInString< unsigned char >
         ( offset + sizeof( unsigned ) );

   // Need a new block
   if ( ++oldSubBlock == blockSize / subBlockSize )
      {
      // TODO
      }
   else
      metaData.SetInString< unsigned char >( oldSubBlock, offset + sizeof( unsigned ) );
   }


void IndexScheduler::IncrementNumBlocks( )
   {
   unsigned oldNum = NumBlocks( );
   metaData.SetInString< unsigned >( oldNum + 1, sizeof( unsigned ) );
   }


