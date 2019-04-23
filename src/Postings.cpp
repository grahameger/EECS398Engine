#include "mmap.h"
#include "vector.h"
#include "PostingList.h"
#include "Postings.h"
#include "ISR.h"

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


// ######################
// Postings Static Variables
// ######################

// 16kB per block
const unsigned Postings::DefaultBlockSize = 16'384;
// 8 sizes on default blocks of 16'384 gives us sizes
// 16'384, 8'192, 4'096, 2'048, 1'024, 512, 256, 128
const unsigned Postings::DefaultNumSizes = 8;
// data reserved for ptr to next
const unsigned Postings::BlockDataOffset = sizeof( unsigned );
// Actual blockSize
unsigned Postings::blockSize = Postings::DefaultBlockSize;
// Default Filename
const char* Postings::Filename = "testIndex";
// The singleton variable
Postings* Postings::CurPostings = nullptr;


// ######################
// Public Postings Functions
// ######################

Postings* Postings::GetPostings( )
   {
   if ( CurPostings == nullptr )
      CurPostings = new Postings( );

   return CurPostings;
   }


void Postings::AddPostings( const FixedLengthString& word, 
      const Vector< unsigned long long >* postings )
   {
   // Get a stringView for where the postingList of this word goes
   SubBlock plSubBlock = GetPostingListSubBlock( word, true );

   // Create the posting list for this word
   PostingList postingList;
   if ( !plSubBlock.uninitialized )
      postingList = PostingList( plSubBlock.ToStringView( ) );

   // Add each posting
   for ( unsigned i = 0; i < postings->size( ); i ++ )
      postingList.AddPosting( postings->at( i ) );

   #ifdef TEST
   printf("Appended %lu postings to %sposting list for %s\n", postings->size( ), 
         plSubBlock.subBlockInfo.blockIndex == 0 ? "new " : "", 
         word.Characters( ) );
   printf("Posting list is now %u bytes.\n", postingList.GetByteSize( ));
   #endif

   // If this is going in a new block
   if ( plSubBlock.subBlockInfo.blockIndex == 0 && 
         postingList.GetByteSize( ) < blockSize - BlockDataOffset )
      {
      plSubBlock = GetNewSubBlock( postingList.GetByteSize( ) );
      postingList.UpdateInPlace( plSubBlock.ToStringView( ) );

      subBlockIndexLock.lock( );
      subBlockIndex.insert( { word, plSubBlock.subBlockInfo } );
      subBlockIndexLock.unlock( );

      wordIndexLock.lock( );
      wordIndex.insert( { plSubBlock.subBlockInfo, word } );
      wordIndexLock.unlock( );

      #ifdef TEST
      printf( "Posting list added to NEW block at (%u, %u) of size %u\n",
            plSubBlock.subBlockInfo.blockIndex,
            plSubBlock.subBlockInfo.subBlockIndex,
            plSubBlock.subBlockInfo.subBlockSize );
      #endif

      MunmapSubBlock( plSubBlock );
      return;
      }

   // Update in place ( if possible ) and return
   if ( postingList.GetByteSize( ) <= plSubBlock.subBlockInfo.subBlockSize )
      {
      postingList.UpdateInPlace( plSubBlock.ToStringView( ) );

      #ifdef TEST
      printf("Putting posting list back in subBlock (%u, %u) of size %u\n",
            plSubBlock.subBlockInfo.blockIndex,
            plSubBlock.subBlockInfo.subBlockIndex,
            plSubBlock.subBlockInfo.subBlockSize );
      #endif

      MunmapSubBlock( plSubBlock );
      return;
      }

   // Split new end posting into x blocks
   auto split = postingList.Split( blockSize - BlockDataOffset );

   // Save the new split postingLists
   SaveSplitPostingList( plSubBlock, split, word );
   }


// #######################
// Private Postings Functions
// #######################

Postings::Postings( ) : indexFD( open( Filename, O_RDWR ) ),
      subBlockIndex( String( Filename ) + "_SBhash" ),
      wordIndex( String( Filename ) + "_Whash" )
   {
   // Failed to open
   if ( indexFD == -1 )
      {
      // Not because file doesn't exist
      if ( errno != ENOENT )
         ErrExit( );

      CreateNewPostingsFile( );
      }

   // Read in the blockSize being used
   if ( read( indexFD, &blockSize, sizeof( unsigned ) ) != sizeof( unsigned ) )
      ErrExit( );
   if ( blockSize != DefaultBlockSize )
      printf( "Warning: using existing blockSize of %u instead of suggested %u\n", 
            blockSize, DefaultBlockSize );
   // Set metaData to the mmapped first block
   metaData = { ( char* )mmapWrapper( indexFD, blockSize, 0 ), blockSize };
   // Second num is numBlocks
   nextBlockIndex = metaData.GetInString< unsigned >( sizeof( unsigned ) );
   }


void Postings::CreateNewPostingsFile( )
   {
   // Create the file
   indexFD = open( Filename, O_RDWR | O_CREAT, S_IRWXU );

   // If couldn't create
   if ( indexFD == -1 )
      ErrExit( );

   // Set it to the correct size (1 for metadata, 1 for each size)
   ftruncate( indexFD, blockSize * ( DefaultNumSizes + 1 ) );

   // Open the first block
   metaData = StringView( ( char* )mmapWrapper( indexFD, blockSize, 0 ), blockSize );
   unsigned currentOffset = 0;

   // Set blockSize in metadata
   metaData.SetInString< unsigned >( blockSize );
   currentOffset += sizeof( unsigned );
   // Set numBlocks in metadata
   metaData.SetInString< unsigned >( DefaultNumSizes, currentOffset );
   currentOffset += sizeof( unsigned );
   // Set numSizes info in metadata
   metaData.SetInString< unsigned >( DefaultNumSizes, currentOffset );

   // Set open Blocks/subBlocks for sizes in metadata
   for ( unsigned block = 1, curSize = blockSize / 2; 
         block != DefaultNumSizes; curSize /= 2, block++ )
      {
      SubBlockInfo sizeInfo{ curSize, block, 0 };
      SetOpen( sizeInfo, true );
      SetLastUsed( { curSize, 0, 0 }, true );
      }

   munmapWrapper( metaData.RawCString( ), metaData.Size( ) );
   }


void Postings::SaveSplitPostingList( SubBlock plSubBlock,
      Vector< PostingList* >& split, const FixedLengthString& word )
   {
   unsigned blockIndexPtr = 0;

   #ifdef TEST
   char Buffer[2048];
   int bufferIndex = 0;

   bufferIndex += sprintf( Buffer, "Putting posting list into %lu blocks:\n",
         split.size( ) );
   #endif

   SubBlock newSubBlock;

   for ( unsigned i = split.size( ); i > 0; i-- )
      {
      // This is a completely new block for the overflow data or
      // The old block is not a full block and needs to be re-allocated
      if ( i > 1 || plSubBlock.subBlockInfo.subBlockSize != blockSize ) 
         {
         newSubBlock = GetNewSubBlock( split[ i - 1 ]->GetByteSize( ) );
         }
      // The old block was a full sized block and just needs to be updated
      else
         {
         split[ 0 ]->UpdateInPlace( plSubBlock.ToStringView( ) );
         plSubBlock.SetNextPtr( blockIndexPtr );

         #ifdef TEST
         sprintf( Buffer + bufferIndex, 
               "(%u, %u) of size %u (The original block)",
               plSubBlock.subBlockInfo.blockIndex,
               plSubBlock.subBlockInfo.subBlockIndex,
               plSubBlock.subBlockInfo.subBlockSize );
         printf( "%s\n", Buffer );
         #endif

         MunmapSubBlock( plSubBlock );
         return;
         }

      split[ i - 1 ]->FullUpdate( newSubBlock.ToStringView( ) );
      if ( newSubBlock.subBlockInfo.subBlockSize == blockSize )
         newSubBlock.SetNextPtr( blockIndexPtr );
      blockIndexPtr = newSubBlock.subBlockInfo.blockIndex;

      #ifdef TEST
      bufferIndex += sprintf( Buffer + bufferIndex, "\t(%u, %u) of size %u\n",
               plSubBlock.subBlockInfo.blockIndex,
               plSubBlock.subBlockInfo.subBlockIndex,
               plSubBlock.subBlockInfo.subBlockSize );
      #endif

      MunmapSubBlock( newSubBlock );
      }

   #ifdef TEST
   printf( "%s\n", Buffer );
   #endif

   // Update subBlockIndex, and wordIndex, then Delete the outgrown subBlock
   assert( plSubBlock.subBlockInfo.subBlockSize != blockSize );

   subBlockIndexLock.lock( );
   if ( plSubBlock.subBlockInfo.blockIndex != 0 )
      subBlockIndex.erase( word );
   subBlockIndex.insert( { word, newSubBlock.subBlockInfo } );
   subBlockIndexLock.unlock( );

   wordIndexLock.lock( );
   if ( plSubBlock.subBlockInfo.blockIndex != 0 )
      wordIndex.erase( newSubBlock.subBlockInfo );
   wordIndex.insert( { newSubBlock.subBlockInfo, word } );
   wordIndexLock.unlock( );

   DeleteSubBlock( plSubBlock );
   }


IsrInfo Postings::GetPostingList( const FixedLengthString& word )
   {
   // Get a stringView for where the postingList of this word goes
   SubBlock plSubBlock = GetPostingListSubBlock( word );
   StringView plStringView = plSubBlock.ToStringView( );

   unsigned nextPtr = plSubBlock.subBlockInfo.subBlockSize != blockSize ? 0 :
         *( unsigned* )plSubBlock.mmappedArea;

   // Create the posting list for this word
   PostingList* postingList;
   if ( !plSubBlock.uninitialized )
      postingList = new PostingList( plStringView );
   else
      postingList = new PostingList( );

   return { nextPtr, postingList, plSubBlock };
   }


IsrInfo Postings::GetPostingList( unsigned blockIndex )
   {
   // Grab that block
   SubBlock plSubBlock = GetSubBlock( { blockSize, blockIndex, 0 }, false, false );
   StringView plStringView = plSubBlock.ToStringView( );

   unsigned nextPtr = *( unsigned* )plSubBlock.mmappedArea;

   PostingList* postingList = new PostingList( plStringView );

   return { nextPtr, postingList, plSubBlock };
   }


SubBlock Postings::GetPostingListSubBlock
      ( const FixedLengthString& word, bool writing )
   {
   subBlockIndexLock.lock( );
   auto wordIt = subBlockIndex.find( word );

   // Return the last block in the chain of blocks stored
   if ( wordIt != subBlockIndex.end( ) ) 
      {
      SubBlockInfo existingInfo = ( *wordIt ).second;
      subBlockIndexLock.unlock( );
      return GetSubBlock( existingInfo, writing, false );
      }
   // Return an empty block
   else
      {
      subBlockIndexLock.unlock( );
      return {true, nullptr, 0, { 0, 0, 0 }, nullptr, true };
      }
   }


SubBlock Postings::GetNewSubBlock( unsigned minSize )
   {
   unsigned actualSize = blockSize;
   while ( actualSize / 2 >= minSize ) { actualSize /= 2; }

   SubBlock subBlock = GetOpenSubBlock( actualSize );
   subBlock.uninitialized = true;

   return subBlock;
   }


SubBlock Postings::GetSubBlock( SubBlockInfo subBlockInfo, bool endWanted, bool writeLockHeld )
   {
   SubBlock subBlock = MmapSubBlock( subBlockInfo, endWanted, writeLockHeld );

   // If we need to follow nextPtrs to get to end
   if ( endWanted && subBlockInfo.subBlockSize == blockSize )
      {
      // While nextBlockPtr != 0
      unsigned nextBlockPtr;
      while ( ( nextBlockPtr = *( unsigned* )subBlock.mmappedArea ) != 0 )
         {
         // Unmap current sub block
         MunmapSubBlock( subBlock );
         // Change to new sub block
         subBlockInfo.blockIndex = nextBlockPtr;
         subBlockInfo.subBlockIndex = 0;
         // Map new sub block
         subBlock = MmapSubBlock( subBlockInfo, endWanted, false );
         }
      }

   return subBlock;
   }


void Postings::DeleteSubBlock( SubBlock subBlock )
   {
   assert( subBlock.subBlockInfo.subBlockSize != blockSize );

   // Get the last used sub block of this size
   SubBlock lastUsed = GetLastUsedSubBlock(
         subBlock.subBlockInfo.subBlockSize, subBlock.subBlockInfo.blockIndex );
   
   #ifdef TEST
   char Buffer[2048];
   int bufferIndex = 0;
   bufferIndex += sprintf( Buffer, "Deleting (%u, %u), lastUsed of size %u is (%u, %u)\n",
         subBlock.subBlockInfo.blockIndex,
         subBlock.subBlockInfo.subBlockIndex,
         subBlock.subBlockInfo.subBlockSize,
         lastUsed.subBlockInfo.blockIndex,
         lastUsed.subBlockInfo.subBlockIndex );
   #endif

   if ( lastUsed.subBlockInfo.blockIndex == 0 )
      {
      #ifdef TEST
      sprintf( Buffer + bufferIndex, "\tNo lastUsed, nothing to move.\n" );
      printf( "%s\n", Buffer );
      #endif

      MunmapSubBlock( subBlock );
      return;
      }

   // This was the last subBlock
   if ( lastUsed.subBlockInfo == subBlock.subBlockInfo )
      {
      #ifdef TEST
      sprintf( Buffer + bufferIndex, "\tDeleting the lastUsed, nothing to move.\n" );
      printf( "%s\n", Buffer );
      #endif

      MunmapSubBlock( subBlock );
      return;
      }

   // Copy it into current subBlock
   memcpy( subBlock.mmappedArea, lastUsed.mmappedArea, 
         lastUsed.subBlockInfo.subBlockSize );

   // Update hashmaps
   wordIndexLock.lock( );
   FixedLengthString word = wordIndex.at( lastUsed.subBlockInfo );
   wordIndex.erase( lastUsed.subBlockInfo );
   wordIndex.at( subBlock.subBlockInfo ) = word;
   wordIndexLock.unlock( );

   subBlockIndexLock.lock( );
   subBlockIndex.at( word ) = subBlock.subBlockInfo;
   subBlockIndexLock.unlock( );

   if ( lastUsed.rwlock )
      MunmapSubBlock( lastUsed );
   MunmapSubBlock( subBlock );
   }


SubBlock Postings::GetOpenSubBlock( unsigned subBlockSize )
   {
   metaDataLock.lock();
   
   // Size of each open/lastused entry
   unsigned offset = sizeof( unsigned ) * 2 + sizeof( unsigned char ) * 2;
   // Indexed based on subBlockSize
   offset *= blockSize / subBlockSize;
   // Plus the original offset from the other data
   offset += sizeof( unsigned ) * 2;

   SubBlockInfo openSubBlock;
   // If we want a whole block, return nextBlockIndex and increment numBlocks
   if ( subBlockSize == blockSize )
      {
      openSubBlock = { subBlockSize, nextBlockIndex, 0 };
      IncrementNumBlocks( true );
      }
   // Otherwise read open subBlock for this size from metaData
   else
      {
      openSubBlock.subBlockSize = subBlockSize;
      openSubBlock.blockIndex = metaData.GetInString< unsigned >( offset );
      offset += sizeof( unsigned );
      openSubBlock.subBlockIndex = metaData.GetInString< unsigned char >( offset );
      }

   // If open is invalid, allocate a new block for this size
   if ( openSubBlock.blockIndex == 0 )
      {
      openSubBlock = { subBlockSize, nextBlockIndex, 0 };
      IncrementNumBlocks( true );
      }

   SubBlockInfo toReturnInfo = openSubBlock;
   SetLastUsed( toReturnInfo, true );

   // If the incremented next subBlock is invalid, make blockIndex invalid too
   if ( ( ++openSubBlock.subBlockIndex %= blockSize / openSubBlock.subBlockSize ) == 0 )
      openSubBlock.blockIndex = 0;

   // Increment open
   SetOpen( openSubBlock, true );

   metaDataLock.unlock();

   // Get the open sub block and set it to the last used for this size
   return MmapSubBlock( toReturnInfo, true, false );
   }


SubBlock Postings::GetLastUsedSubBlock( unsigned subBlockSize, unsigned blockHeld )
   {
   // Should never be called for blockSize
   assert( subBlockSize != blockSize );

   metaDataLock.lock();
   // Size of each open/lastused entry
   unsigned offset = sizeof( unsigned ) * 2 + sizeof( unsigned char ) * 2;
   // Indexed based on subBlockSize
   offset *= blockSize / subBlockSize;
   // Plus the original offset from the other data
   offset += sizeof( unsigned ) * 3 + sizeof( unsigned char );

   SubBlockInfo lastUsedSubBlock;

   lastUsedSubBlock.subBlockSize = subBlockSize;
   lastUsedSubBlock.blockIndex = metaData.GetInString< unsigned >( offset );
   offset += sizeof( unsigned );
   lastUsedSubBlock.subBlockIndex = metaData.GetInString< unsigned char >( offset );

   // If lastUsedSubBlock is invalid
   if ( lastUsedSubBlock.blockIndex == 0 )
      {
      metaDataLock.unlock( );
      return { true, nullptr, 0, { 0, 0, 0 }, nullptr, true };
      }

   SubBlockInfo toReturnInfo = lastUsedSubBlock;
   SetOpen( lastUsedSubBlock, true );

   // If decremented lastUsedSubBlock is invalid
   if ( lastUsedSubBlock.subBlockIndex-- == 0 )
      {
      lastUsedSubBlock.subBlockIndex = 0;
      lastUsedSubBlock.blockIndex = 0;
      }

   // Decrement lastUsedSubBlock
   SetLastUsed( lastUsedSubBlock, true );

   metaDataLock.unlock();

   return MmapSubBlock( toReturnInfo, true, toReturnInfo.blockIndex == blockHeld );
   }


inline void Postings::IncrementNumBlocks( bool metaDataHeld )
   {
   if ( !metaDataHeld )
      metaDataLock.lock();
   metaData.SetInString< unsigned >( ++nextBlockIndex, sizeof( unsigned ) );

   if ( !metaDataHeld )
      metaDataLock.unlock();
   }


void Postings::SetLastUsed( SubBlockInfo lastUsed, bool metaDataHeld )
   {
   // Size of each open/lastused entry
   unsigned offset = sizeof( unsigned ) * 2 + sizeof( unsigned char ) * 2;
   // Indexed based on subBlockSize
   offset *= blockSize / lastUsed.subBlockSize;
   // Plus the original offset from the other data
   offset += sizeof( unsigned ) * 3 + sizeof( unsigned char );

   if ( !metaDataHeld )
      metaDataLock.lock();

   // Set last used block for size subBlockSize
   metaData.SetInString< unsigned >( lastUsed.blockIndex, offset );
   offset += sizeof( unsigned );
   // Set last used subBlock for size subBlockSize
   metaData.SetInString< unsigned char >( lastUsed.subBlockIndex, offset );

   if ( !metaDataHeld )
      metaDataLock.unlock();
   }


void Postings::SetOpen( SubBlockInfo open, bool metaDataHeld )
   {
   if ( open.subBlockSize == blockSize )
      return;

   // Size of each open/lastused entry
   unsigned offset = sizeof( unsigned ) * 2 + sizeof( unsigned char ) * 2;
   // Indexed based on subBlockSize
   offset *= blockSize / open.subBlockSize;
   // Plus the original offset from the other data
   offset += sizeof( unsigned ) * 2;

   if ( !metaDataHeld )
      metaDataLock.lock();
   // Set open block for size subBlockSize
   metaData.SetInString< unsigned >( open.blockIndex, offset );
   offset += sizeof( unsigned );
   // Set open subBlock for size subBlockSize
   metaData.SetInString< unsigned char >( open.subBlockIndex, offset );

   if ( !metaDataHeld )
      metaDataLock.unlock();
   }


SubBlock Postings::MmapSubBlock( SubBlockInfo subBlockInfo, bool writing, bool writeLockHeld )
   {
   // BlockIndex * blockSize + subBlockIndex * subBlockSize
   unsigned blockOffset = subBlockInfo.blockIndex * blockSize;
   unsigned subBlockOffset = subBlockInfo.subBlockIndex * subBlockInfo.subBlockSize;

   unsigned dataOffset = 
         subBlockInfo.subBlockSize == blockSize ? BlockDataOffset : 0;

   SubBlock toReturn = { false, nullptr, dataOffset, subBlockInfo, nullptr, writing };

   if ( !writeLockHeld )
      {
      try
         { 
         lockMapLock.lock();
         toReturn.rwlock = lockMap.at( subBlockInfo.blockIndex ); 
         lockMapLock.unlock();
         #ifdef TEST
         printf( "Found a lock for block %d. ",
               subBlockInfo.blockIndex );
         #endif
         }
      catch ( const std::out_of_range& )
         { 
         toReturn.rwlock = new threading::ReadWriteLock( );
         lockMap.insert( { subBlockInfo.blockIndex, toReturn.rwlock } );
         lockMapLock.unlock();
         #ifdef TEST
         printf( "No lock found for block %d. Making a new one. ", 
               subBlockInfo.blockIndex );
         #endif
         }

      #ifdef TEST
      printf( "Getting a %s lock on it.\n", writing ? "write" : "read" );
      #endif

      if ( writing )
         toReturn.rwlock->writeLock( );
      else
         toReturn.rwlock->readLock( );
      }

   #ifdef TEST
   printf( "Lock acquired for %d.\n", subBlockInfo.blockIndex );
   #endif

   toReturn.mmappedArea = 
         ( char* )mmapWrapper( indexFD, blockSize, blockOffset );
   toReturn.mmappedArea += subBlockOffset;

   return toReturn;
   }


void Postings::MunmapSubBlock( SubBlock subBlock )
   {
   char* mmappedArea = subBlock.mmappedArea -
         subBlock.subBlockInfo.subBlockIndex * subBlock.subBlockInfo.subBlockSize;

   munmapWrapper( mmappedArea, blockSize );
   subBlock.mmappedArea = nullptr;

   if ( subBlock.rwlock == nullptr )
      { 
      printf("Unlocking a lock that doesn't exist!\n");
      exit( 1 );
      }

   #ifdef TEST
   printf("Unlocking the %s lock on %d.\n", 
         subBlock.writingLock ? "write" : "read",
         subBlock.subBlockInfo.blockIndex );
   #endif

   subBlock.rwlock->unlock( );
   }


unsigned Postings::SmallestSubBlockSize( )
   {
   metaDataLock.lock( );
   unsigned numSizes = metaData.GetInString< unsigned >( 2 * sizeof( unsigned ) );
   metaDataLock.unlock();
   return blockSize >> ( numSizes - 1 );
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

FixedLengthString::FixedLengthString()
   {
      characters[0] = 0;
      characters[MaxLength] = 0;
   }

FixedLengthURL::FixedLengthURL( const char* cstring )
   {
   strncpy( characters, cstring, MaxLength );
   characters[ MaxLength ] = 0;
   }


bool FixedLengthURL::operator== ( const FixedLengthURL& other ) const
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

FixedLengthURL::FixedLengthURL()
   {
      characters[0] = 0;
      characters[MaxLength] = 0;
   }
// SubBlock

inline void SubBlock::SetNextPtr( unsigned blockIndex )
   {
   *( unsigned* )mmappedArea = blockIndex;
   }


inline bool SubBlock::operator!= ( const SubBlock& other ) const
   {
   return subBlockInfo.blockIndex != other.subBlockInfo.blockIndex ||
         subBlockInfo.subBlockIndex != other.subBlockInfo.subBlockIndex;
   }


inline StringView SubBlock::ToStringView( )
   {
   return { mmappedArea + dataOffset, subBlockInfo.subBlockSize - dataOffset };
   }
