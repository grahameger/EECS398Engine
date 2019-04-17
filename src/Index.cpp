#include "mmap.h"
#include "vector.h"
#include "PostingList.h"
#include "Index.h"
// For open( )
#include <fcntl.h>
// For errno
#include <errno.h>
// For stderr
#include <stdio.h>
// For strerror
#include <string.h>
// For exit
#include <stdlib.h>
// For read
#include <unistd.h>
// For assert
#include <assert.h>

// ################
// Helper Functions
// ################

void ErrExit( )
   {
   fprintf( stderr, "%s\n", strerror( errno ) );
   exit( 1 );
   }


// #####################
// Other Data Structures
// #####################

// PostingListSubBlock

struct PostingListSubBlock
   {
   char* subBlock;
   SubBlockInfo subBlockInfo;
   PostingList postingList;
   };


// ######################
// Index Static Variables
// ######################

// 16kB per block
const unsigned Index::DefaultBlockSize = 16'384;
// 8 sizes on default blocks of 16'384 gives us sizes
// 16'384, 8'192, 4'096, 2'048, 1'024, 512, 256, 128
const unsigned Index::DefaultNumSizes = 8;


// ######################
// Public Index Functions
// ######################

Index::Index( const char* filename, unsigned recSize, unsigned numSizes )
      : indexFD( open( filename, O_RDWR ) ),
      subBlockIndex( String( filename ) + "_hash" )
   {
   // Failed to open
   if ( indexFD == -1 )
      {
      // Not because file doesn't exist
      if ( errno != ENOENT )
         ErrExit( );

      CreateNewIndexFile( filename, recSize, numSizes );
      }

   // Read in the blockSize being used
   if ( read( indexFD, &blockSize, sizeof( unsigned ) ) != sizeof( unsigned ) )
      ErrExit( );
   if ( blockSize != recSize )
      printf( "Warning: using existing blockSize instead of suggested\n" );
   // Set metaData to the mmapped first block
   metaData = { ( char* )mmapWrapper( indexFD, blockSize, 0 ), blockSize };
   // Second num is numBlocks
   nextBlockIndex = metaData.GetInString< unsigned >( sizeof( unsigned ) );
   }


void Index::AddPostings( const FixedLengthString& word, 
      const Vector< unsigned long long >* postings )
   {
   PostingListSubBlock plSubBlock = GetPostingList( word, true );
   // Add each posting
   // Update in place ( if possible ) and return
   // split new end posting into x blocks
   // add x blocks backwards pointing to each other
   }


// #######################
// Private Index Functions
// #######################

void Index::CreateNewIndexFile( const char* filename, unsigned blockSize, 
      unsigned numSizes )
   {
   // Create the file
   indexFD = open( filename, O_RDWR | O_CREAT, S_IRWXU );

   // If couldn't create
   if ( indexFD == -1 )
      ErrExit( );

   // Set it to the correct size (1 for metadata, 1 for each size)
   ftruncate( indexFD, blockSize * ( numSizes + 1 ) );

   // Open the first block
   StringView data( ( char* )mmapWrapper( indexFD, blockSize, 0 ), blockSize );
   unsigned currentOffset = 0;

   // Set blockSize in metadata
   data.SetInString< unsigned >( blockSize );
   currentOffset += sizeof( unsigned );
   // Set numBlocks in metadata
   data.SetInString< unsigned >( numSizes + 1, currentOffset );
   currentOffset += sizeof( unsigned );
   // Set numSizes info in metadata
   data.SetInString< unsigned >( numSizes, currentOffset );

   // Set open Blocks/subBlocks for sizes in metadata
   for ( unsigned block = 1, curSize = blockSize; 
         block != numSizes + 1; curSize >>= 1, block++ )
      {
      unsigned offset = sizeof( unsigned ) +
            ( blockSize / curSize * sizeof( unsigned ) * 2 );
      // Set block for size curSize
      data.SetInString< unsigned >( block, offset );
      offset += sizeof( unsigned );
      // Set subBlock for size curSize
      data.SetInString< unsigned char >( 0, offset );
      }

   munmapWrapper( data.RawCString( ), data.Size( ) );
   }


PostingListSubBlock Index::GetPostingList
      ( const FixedLengthString& word, bool endWanted )
   {
   auto wordIt = subBlockIndex.find( word );

   // Return the last block in the chain of blocks stored
   if ( wordIt != subBlockIndex.end( ) )
      {
      printf("Word already found at Block: %u, SubBlock: %u, with Size: %u\n", ( *wordIt ).second.blockIndex, ( *wordIt ).second.subBlockIndex, ( *wordIt ).second.subBlockSize );
      return GetPostingListSubBlock( ( *wordIt ).second, endWanted );
      }
   // Return a new block
   else
      return GetNewPostingListSubBlock( word );
   }


PostingListSubBlock Index::GetNewPostingListSubBlock( const FixedLengthString& word )
   {
   SubBlockInfo subBlockInfo = GetOpenSubBlock( SmallestSubBlockSize( ) );
   // TODO
   IncrementOpenSubBlock( SmallestSubBlockSize( ) );

   subBlockIndex.insert( { word, subBlockInfo } );

   return { MmapSubBlock( subBlockInfo ), subBlockInfo, PostingList( ) };
   }


PostingListSubBlock Index::GetPostingListSubBlock
      ( SubBlockInfo subBlockInfo, bool endWanted )
   {
   char* subBlock = MmapSubBlock( subBlockInfo );

   unsigned postingListSize = subBlockInfo.subBlockSize;
   unsigned offset = 0;

   // If we need to follow nextPtrs to get to end
   if ( endWanted && subBlockInfo.subBlockSize == blockSize )
      {
      // We now have an offset / smaller size because of nextPtr
      offset = sizeof( unsigned ) + sizeof( unsigned char );
      postingListSize -= offset;

      // While nextBlockPtr != 0
      unsigned nextBlockPtr;
      while ( ( nextBlockPtr = *( unsigned* )subBlock ) != 0 )
         {
         // Grab the next sub block ptr too
         unsigned nextSubBlockPtr = 
               *( unsigned char* )( subBlock + sizeof( unsigned ) );

         // Unmap current sub block
         MunmapSubBlock( subBlockInfo, subBlock );
         // Change to new sub block
         subBlockInfo.blockIndex = nextBlockPtr;
         subBlockInfo.subBlockIndex = nextSubBlockPtr;
         // Map new sub block
         subBlock = MmapSubBlock( subBlockInfo );
         }
      }

   // Get a StringView of the bytes ( minus nextPtr if it exists )
   StringView postingListString( subBlock + offset, postingListSize );

   return { subBlock, subBlockInfo, PostingList( postingListString ) };
   }


unsigned Index::SmallestSubBlockSize( ) const
   {
   unsigned numSizes = metaData.GetInString< unsigned >( 2 * sizeof( unsigned ) );
   return blockSize >> ( numSizes - 1 );
   }


SubBlockInfo Index::GetOpenSubBlock( unsigned subBlockSize ) const
   {
   unsigned offset = sizeof( unsigned ) +
         ( blockSize / subBlockSize * sizeof( unsigned ) * 2 );

   SubBlockInfo returnInfo;

   returnInfo.subBlockSize = subBlockSize;
   returnInfo.blockIndex = metaData.GetInString< unsigned >( offset );
   offset += sizeof( unsigned );
   returnInfo.subBlockIndex = metaData.GetInString< unsigned char >( offset );

   return returnInfo;
   }


void Index::IncrementOpenSubBlock( unsigned subBlockSize )
   {
   }


char* Index::MmapSubBlock( SubBlockInfo subBlockInfo )
   {
   // BlockIndex * blockSize + subBlockIndex * subBlockSize
   unsigned offset = subBlockInfo.blockIndex * blockSize + 
         subBlockInfo.subBlockIndex * subBlockInfo.subBlockSize;

   return ( char* )mmapWrapper( indexFD, subBlockInfo.subBlockSize, offset );
   }


inline void Index::MunmapSubBlock( SubBlockInfo subBlockInfo, char* subBlock )
   {
   munmapWrapper( subBlock, subBlockInfo.subBlockSize );
   }


// ####################################
// Other Data Structures Implementation
// ####################################

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


