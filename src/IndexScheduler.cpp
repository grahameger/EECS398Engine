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

// FixedLengthString

FixedLengthString::FixedLengthString( const char* cstring )
   {
   strncpy( characters, cstring, MaxLength );
   characters[ MaxLength ] = 0;
   }


bool FixedLengthString::operator== ( const FixedLengthString& other ) const
   {
   for ( unsigned i = 0; i < MaxLength; i++ )
      {
      if ( characters[ i ] != other.characters[ i ] )
         return false;
      if ( characters[ i ] == 0 )
         return true;
      }
   return true;
   }


// SchedulerSubBlock

SchedulerSubBlock::SchedulerSubBlock( SubBlockInfo subBlockInfo, bool initialized )
      : blockIndex( subBlockInfo.second.first ), 
      subBlockIndex( subBlockInfo.second.second ), blockSize( subBlockInfo.first ),
      initialized( initialized ), inMemory( false )
   { }


SchedulerSubBlock::~SchedulerSubBlock( )
   {
   }


bool SchedulerSubBlock::GetInitialized( ) const
   { return initialized; }

unsigned SchedulerSubBlock::GetBlockSize( ) const
   { return blockSize; }

unsigned SchedulerSubBlock::GetBlockIndex( ) const
   { return blockIndex; }

unsigned char SchedulerSubBlock::GetSubBlockIndex( ) const
   { return subBlockIndex; }

StringView SchedulerSubBlock::GetStringView( )
   {
   IndexScheduler* scheduler = IndexScheduler::GetScheduler( );
   return scheduler->GetSubBlockString( SubBlockLoc( blockIndex, subBlockIndex ) );
   }


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


// TODO: find returns correctly on first run (added assert after insert and it passed)
// but on subsequent runs of the same stuff, it doesn't...
SchedulerSubBlock IndexScheduler::GetPostingList( const FixedLengthString& word )
   {
   auto wordIt = subBlockIndex.find( word );

   // word found
   if ( wordIt != subBlockIndex.end( ) )
      {
      SubBlockInfo subBlockInfo = (*wordIt).second;
      return SchedulerSubBlock( subBlockInfo );
      }

   // word not found
   SubBlockInfo subBlockInfo = GetOpenSubBlock( SmallestSubBlockSize( ) );
   subBlockIndex.insert( Pair( word, subBlockInfo ) );

   IncrementOpenSubBlock( SmallestSubBlockSize( ) );

   return SchedulerSubBlock( subBlockInfo, false );
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


// TODO: Write map data structure
StringView IndexScheduler::GetSubBlockString( SubBlockLoc subBlockLoc )
   {
   return { nullptr, 0 };
   }


SubBlockInfo IndexScheduler::GetOpenSubBlock( unsigned subBlockSize )
   {
   unsigned offset = sizeof( unsigned ) +
         ( blockSize / subBlockSize * sizeof( unsigned ) * 2 );

   SubBlockInfo returnInfo;

   returnInfo.second.first = metaData.GetInString< unsigned >( offset );
   offset += sizeof( unsigned );
   returnInfo.second.second = metaData.GetInString< unsigned char >( offset );
   returnInfo.first = subBlockSize;

   return returnInfo;
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


