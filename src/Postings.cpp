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


#ifdef TEST
// Debug variables
thread_local const FixedLengthString* curWord;
thread_local char Buffer[ 4096 ];
thread_local unsigned bufferIndex = 0;

// Reset the debug varialbes
#define RESETDEBUG( newWord ) curWord = newWord; bufferIndex = 0;                       \
      printf( "Resetting debug info, word is now: %s, bufferIndex is %u\n",             \
      curWord->Characters( ), bufferIndex );

// Print the current buffer
#define PRINTDEBUG( ) printf( "%s\n", Buffer )

// Add to debug buffer
#define DEBUG( args... )                                                                \
      {                                                                                 \
      if ( bufferIndex > 4000 )                                                         \
         {                                                                              \
         PRINTDEBUG( );                                                                 \
         RESETDEBUG( curWord );                                                                 \
         }                                                                              \
      bufferIndex += sprintf( Buffer + bufferIndex, "(%s)\t", curWord->Characters( ) ); \
      bufferIndex += sprintf( Buffer + bufferIndex, args );                             \
      }

#else
// Get rid of the debug lines
#define RESETDEBUG( newWord )
#define PRINTDEBUG( )
#define DEBUG( args... )

#endif


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
threading::Mutex Postings::constructorMutex;


// ######################
// Public Postings Functions
// ######################

Postings* Postings::GetPostings( )
   {
   constructorMutex.lock( );
   if ( CurPostings == nullptr )
      CurPostings = new Postings( );
   constructorMutex.unlock( );

   return CurPostings;
   }


void Postings::AddPostings( const FixedLengthString& word, 
      const Vector< unsigned long long >* postings )
   {
   wordModifyLock.lock( );
   auto wordModifyIt = wordModifyMap.find( word );
   
   threading::Mutex* wordLock;
   if ( wordModifyIt != wordModifyMap.end( ) )
      wordLock = ( *wordModifyIt ).second;
   else
      {
      wordLock = new threading::Mutex( );
      // assert( !( wordModifyMap.find( word ) != wordModifyMap.end( ) ) );
      wordModifyMap.insert( { word, wordLock } );
      }
   wordModifyLock.unlock( );

   wordLock->lock( );

   RESETDEBUG( &word );

   SubBlock plSubBlock = GetPostingListSubBlock( word, true );

   // Create the posting list for this word
   PostingList postingList;
   if ( !plSubBlock.uninitialized )
      postingList = PostingList( plSubBlock.ToStringView( ) );
   // Add each posting
   for ( unsigned i = 0; i < postings->size( ); i ++ )
      postingList.AddPosting( postings->at( i ) );


   DEBUG( "Appended %lu postings to %sposting list\n", postings->size( ), 
         plSubBlock.subBlockInfo.blockIndex ? "" : "new " );
   DEBUG( "Posting list is now %u bytes.\n", postingList.GetByteSize( ) );

   // If this is going in a new block
   if ( plSubBlock.uninitialized && 
         postingList.GetByteSize( ) < plSubBlock.subBlockInfo.subBlockSize )
      {
      postingList.UpdateInPlace( plSubBlock.ToStringView( ) );

      /*
      subBlockIndexLock.lock( );
      assert( !( subBlockIndex.find( word ) != subBlockIndex.end( ) ) );
      subBlockIndex.insert( { word, plSubBlock.subBlockInfo } );
      subBlockIndexLock.unlock( );

      wordIndexLock.lock( );
      assert( wordIndex.find( plSubBlock.subBlockInfo ) != wordIndex.end( ) );
      // wordIndex.insert( { plSubBlock.subBlockInfo, word } );
      wordIndexLock.unlock( );
      */

      DEBUG( "Posting list added to NEW block at (%u, %u) of size %u\n",
            plSubBlock.subBlockInfo.blockIndex,
            plSubBlock.subBlockInfo.subBlockIndex,
            plSubBlock.subBlockInfo.subBlockSize );

      MunmapSubBlock( plSubBlock );

      PRINTDEBUG( );
      wordLock->unlock( );

      return;
      }

   // Update in place ( if possible ) and return
   if ( postingList.GetByteSize( ) <= plSubBlock.subBlockInfo.subBlockSize )
      {

      /*
      wordIndexLock.lock( );
      // Subblock has a mapping, or is blockSize ( could be middle of nextPtr chain )
      assert( wordIndex.find( plSubBlock.subBlockInfo ) != wordIndex.end( ) ||
            plSubBlock.subBlockInfo.subBlockSize == blockSize );
      wordIndexLock.unlock( );

      subBlockIndexLock.lock( );
      assert( subBlockIndex.find( word ) != subBlockIndex.end( ) );
      subBlockIndexLock.unlock( );
      */

      postingList.UpdateInPlace( plSubBlock.ToStringView( ) );

      DEBUG( "Putting posting list back in subBlock (%u, %u) of size %u\n",
            plSubBlock.subBlockInfo.blockIndex,
            plSubBlock.subBlockInfo.subBlockIndex,
            plSubBlock.subBlockInfo.subBlockSize );

      MunmapSubBlock( plSubBlock );

      PRINTDEBUG( );
      wordLock->unlock( );

      return;
      }

   // Split new end posting into x blocks
   auto split = postingList.Split( blockSize - BlockDataOffset );

   // Save the new split postingLists
   SaveSplitPostingList( plSubBlock, split, word );

   PRINTDEBUG( );
   wordLock->unlock( );
   }


// ##########################
// Private Postings Functions
// ##########################

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

   assert( blockSize != 0 );
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

   assert( blockSize != 0 );
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

   DEBUG( "Putting posting list into %lu blocks:\n", split.size( ) );

   SubBlock newSubBlock = { true, nullptr, 0, { 0, 0, 0 }, nullptr, true };

   for ( unsigned i = split.size( ); i > 0; i-- )
      {
      // This is a completely new block for the overflow data
      if ( i > 1 || ( split.size( ) > 1 && plSubBlock.subBlockInfo.subBlockSize != blockSize ) )
         newSubBlock = GetNewSubBlock( blockSize );
      // The old block is not a full block and needs to be re-allocated
      else if ( plSubBlock.subBlockInfo.subBlockSize != blockSize ) 
         newSubBlock = GetNewSubBlock( split[ i - 1 ]->GetByteSize( ) );
      // The old block was a full sized block and just needs to be updated
      else
         {
         split[ 0 ]->UpdateInPlace( plSubBlock.ToStringView( ) );
         plSubBlock.SetNextPtr( blockIndexPtr );

	 /*
	 wordIndexLock.lock( );
	 assert( wordIndex.find( plSubBlock.subBlockInfo ) != wordIndex.end( ) );
	 wordIndexLock.unlock( );
	 */

         DEBUG( "\t(%u, %u) of size %u (The original block)\n",
               plSubBlock.subBlockInfo.blockIndex,
               plSubBlock.subBlockInfo.subBlockIndex,
               plSubBlock.subBlockInfo.subBlockSize );

         MunmapSubBlock( plSubBlock );
         return;
         }

      split[ i - 1 ]->FullUpdate( newSubBlock.ToStringView( ) );
      if ( newSubBlock.subBlockInfo.subBlockSize == blockSize )
         newSubBlock.SetNextPtr( blockIndexPtr );
      blockIndexPtr = newSubBlock.subBlockInfo.blockIndex;

      DEBUG( "\t(%u, %u) of size %u\n",
            newSubBlock.subBlockInfo.blockIndex,
            newSubBlock.subBlockInfo.subBlockIndex,
            newSubBlock.subBlockInfo.subBlockSize );

      // If this was a blockSize update, re-map the word->subBlockInfo
      if ( i == 1 )
         {
	 
         DEBUG( "Reached the questionable statement\n" );
         wordIndexLock.lock( );
         // assert( !( wordIndex.find( newSubBlock.subBlockInfo ) != wordIndex.end( ) ) );
         wordIndex.insert( { newSubBlock.subBlockInfo, word } );
         // assert( wordIndex.at( newSubBlock.subBlockInfo ) == word );
         wordIndexLock.unlock( );

         subBlockIndexLock.lock( );
         // Create if not there ( new postings list > blockSize )
         // Or it is already there ( outgrew subBlock )
         subBlockIndex.at( word ) = newSubBlock.subBlockInfo;
         subBlockIndexLock.unlock( );

	 DEBUG( "Maps updated\n" );
         }

      MunmapSubBlock( newSubBlock );
      }

   // Delete the outgrown subBlock
   assert( plSubBlock.subBlockInfo.subBlockSize != blockSize );

   if ( plSubBlock.mmappedArea != nullptr )
      DeleteSubBlock( plSubBlock );
   }


IsrInfo Postings::GetPostingList( const FixedLengthString& word )
   {
   SubBlock plSubBlock = GetPostingListSubBlock( word, false );

   unsigned nextPtr = plSubBlock.subBlockInfo.subBlockSize != blockSize ? 0 :
         *( unsigned* )plSubBlock.mmappedArea;

   // Create the posting list for this word
   PostingList* postingList;
   if ( !plSubBlock.uninitialized )
      postingList = new PostingList( plSubBlock.ToStringView( ) );
   else
      postingList = nullptr;

   return { nextPtr, postingList, plSubBlock };
   }


IsrInfo Postings::GetPostingList( unsigned blockIndex )
   {
   // Grab that block
   SubBlock plSubBlock = GetSubBlock( { blockSize, blockIndex, 0 }, false, false );

   unsigned nextPtr = *( unsigned* )plSubBlock.mmappedArea;

   PostingList* postingList = new PostingList( plSubBlock.ToStringView( ) );

   return { nextPtr, postingList, plSubBlock };
   }


// TODO: Find a better way to ensure that the return SubBlock is still
// the subBlockIndex for this word, because spinning until they match could
// POTENTIALLY run forever
SubBlock Postings::GetPostingListSubBlock
      ( const FixedLengthString& word, bool writing )
   {
   SubBlock toReturn = { true, nullptr, 0, { 0, 0, 0 }, nullptr, true };
   bool notMatched = true;

   do {
      subBlockIndexLock.lock( );
      auto wordIt = subBlockIndex.find( word );

      // word has a currentSubBlock
      if ( wordIt != subBlockIndex.end( ) )
         {
         SubBlockInfo existingInfo = ( *wordIt ).second;
         subBlockIndexLock.unlock( );

	 if ( existingInfo.blockIndex == 0 )
            continue;

         // we have the correct subBlock
         if ( toReturn.subBlockInfo == existingInfo || 
	       toReturn.subBlockInfo.subBlockSize == blockSize )
            notMatched = false;
         // we need a new subBlock
         else
            {
            // unmap what we had
            if ( toReturn.rwlock != nullptr )
               MunmapSubBlock( toReturn );
            // grab the new one
	    toReturn = GetSubBlock( existingInfo, writing, false );
	    }
	 }
      // word has no currentSubBlock
      else
         {
         subBlockIndex.insert( { word, { 0, 0, 0 } } );
         subBlockIndexLock.unlock( );

	 if ( !writing )
	    return { true, nullptr, 0, { 0, 0, 0 }, nullptr, true };
         toReturn = GetNewSubBlock( SmallestSubBlockSize( ) );

         subBlockIndexLock.lock( );
         subBlockIndex.at( word ) = toReturn.subBlockInfo;
         subBlockIndexLock.unlock( );

	 wordIndexLock.lock( );
	 // assert( !( wordIndex.find( toReturn.subBlockInfo ) != wordIndex.end( ) ) );
	 wordIndex.insert( { toReturn.subBlockInfo, word } );
	 wordIndexLock.unlock( );

         return toReturn;
         }
      } while( notMatched );

   return toReturn;
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
   if ( subBlockInfo.blockIndex == 0 )
      return { true, nullptr, 0, { 0, 0, 0 }, nullptr, true };

   SubBlock subBlock = MmapSubBlock( subBlockInfo, endWanted, writeLockHeld );

   // If we need to follow nextPtrs to get to end
   if ( endWanted && subBlockInfo.subBlockSize == blockSize )
      {
      // While nextBlockPtr != 0
      unsigned nextBlockPtr;
      while ( ( nextBlockPtr = *( unsigned* )subBlock.mmappedArea ) != 0 )
         {
         // Unmap current sub block
         if ( subBlock.rwlock != nullptr )
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

   // Remove that subBlock from the wordIndex
   wordIndexLock.lock( );
   wordIndex.erase( subBlock.subBlockInfo );
   wordIndexLock.unlock( );

   // Get the last used sub block of this size
   SubBlock lastUsed = GetLastUsedSubBlock(
         subBlock.subBlockInfo.subBlockSize, subBlock.subBlockInfo );
   
   DEBUG( "Deleting (%u, %u), lastUsed of size %u is (%u, %u)\n",
         subBlock.subBlockInfo.blockIndex,
         subBlock.subBlockInfo.subBlockIndex,
         subBlock.subBlockInfo.subBlockSize,
         lastUsed.subBlockInfo.blockIndex,
         lastUsed.subBlockInfo.subBlockIndex );
         
   if ( lastUsed.subBlockInfo.blockIndex == 0 )
      {
      DEBUG( "\tNo lastUsed, nothing to move. \n" );

      assert( lastUsed.rwlock == nullptr );
      MunmapSubBlock( subBlock );
      return;
      }

   // This was the last subBlock
   if ( lastUsed.subBlockInfo == subBlock.subBlockInfo )
      {
      DEBUG( "\tDeleting the lastUsed, nothing to move.\n" );

      assert( lastUsed.rwlock == nullptr );
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
   // assert( !( wordIndex.find( subBlock.subBlockInfo ) != wordIndex.end( ) ) );
   wordIndex.insert( { subBlock.subBlockInfo, word } );
   wordIndexLock.unlock( );

   subBlockIndexLock.lock( );
   // assert( subBlockIndex.at( word ) == lastUsed.subBlockInfo );
   subBlockIndex.at( word ) = subBlock.subBlockInfo;
   subBlockIndexLock.unlock( );

   if ( lastUsed.rwlock != nullptr )
      MunmapSubBlock( lastUsed );
   MunmapSubBlock( subBlock );
   }


SubBlock Postings::GetOpenSubBlock( unsigned subBlockSize )
   {

   bool acquiredOpen = false;
   SubBlock toReturn = { true, nullptr, 0, { 0, 0, 0 }, nullptr, true };

   do {
      bool incrementedPage = false;

      SubBlockInfo openInfo;

      metaDataLock.lock();

      // If we want a whole block, return nextBlockIndex and increment numBlocks
      if ( subBlockSize == blockSize )
         {
         openInfo = { subBlockSize, nextBlockIndex, 0 };
         incrementedPage = true;
         }
      else
         {
         // Size of each open/lastused entry
         unsigned offset = sizeof( unsigned ) * 2 + sizeof( unsigned char ) * 2;
         // Indexed based on subBlockSize
         offset *= blockSize / subBlockSize;
         // Plus the original offset from the other data
         offset += sizeof( unsigned ) * 2;

         openInfo.subBlockSize = subBlockSize;
         openInfo.blockIndex = metaData.GetInString< unsigned >( offset );
         offset += sizeof( unsigned );
         openInfo.subBlockIndex = metaData.GetInString< unsigned char >( offset );

         // If open is invalid, allocate a new block for this size
         if ( openInfo.blockIndex == 0 )
            {
            openInfo = { subBlockSize, nextBlockIndex, 0 };
            incrementedPage = true;
            }
         }
      
      // We have the right open subBlock
      if ( openInfo == toReturn.subBlockInfo )
         {
         if ( openInfo.subBlockSize != blockSize )
            {
            SetLastUsed( openInfo, true );
            // If the incremented next subBlock is invalid, make blockIndex invalid too
            if ( ( ++openInfo.subBlockIndex %= blockSize / openInfo.subBlockSize ) == 0 )
               openInfo.blockIndex = 0;
            SetOpen( openInfo, true );
            }

         if ( incrementedPage )
            IncrementNumBlocks( true );
         metaDataLock.unlock( );

         return toReturn;
         }

      // unmap what we had
      if ( toReturn.rwlock != nullptr )
         MunmapSubBlock( toReturn );
      metaDataLock.unlock( );

      // Get the open sub block and set it to the last used for this size
      toReturn = MmapSubBlock( openInfo, true, false );
      } while ( !acquiredOpen );

   return toReturn;
   }


SubBlock Postings::GetLastUsedSubBlock( unsigned subBlockSize, SubBlockInfo subBlockHeld )
   {
   // Should never be called for blockSize
   assert( subBlockSize != blockSize );

   bool acquiredLastUsed = false;
   SubBlock toReturn = { true, nullptr, 0, { 0, 0, 0 }, nullptr, true };

   do {
      metaDataLock.lock();

      SubBlockInfo lastUsedInfo;

      // Size of each open/lastused entry
      unsigned offset = sizeof( unsigned ) * 2 + sizeof( unsigned char ) * 2;
      // Indexed based on subBlockSize
      offset *= blockSize / subBlockSize;
      // Plus the original offset from the other data
      offset += sizeof( unsigned ) * 3 + sizeof( unsigned char );

      lastUsedInfo.subBlockSize = subBlockSize;
      lastUsedInfo.blockIndex = metaData.GetInString< unsigned >( offset );
      offset += sizeof( unsigned );
      lastUsedInfo.subBlockIndex = metaData.GetInString< unsigned char >( offset );

      // If lastUsedSubBlock is invalid, return a dummy because there is no lastUsed
      if ( lastUsedInfo.blockIndex == 0 )
         {
         metaDataLock.unlock( );

         if ( toReturn.rwlock != nullptr )
            MunmapSubBlock( toReturn );

         return { true, nullptr, 0, lastUsedInfo, nullptr, true };
         }

      // If we've acquired the correct subBlock
      if ( toReturn.subBlockInfo == lastUsedInfo )
         {
         SetOpen( lastUsedInfo, true );

         if ( lastUsedInfo.subBlockIndex-- == 0 )
            lastUsedInfo.blockIndex = 0;

         SetLastUsed( lastUsedInfo, true );

         metaDataLock.unlock( );
         return toReturn;
         }

      // unmap what we had
      if ( toReturn.rwlock != nullptr )
         MunmapSubBlock( toReturn );
      metaDataLock.unlock();

      toReturn = MmapSubBlock( lastUsedInfo, true, lastUsedInfo == subBlockHeld );
   } while ( !acquiredLastUsed );

   return toReturn;
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
      lockMapLock.lock();
      if ( lockMap.find( subBlockInfo ) != lockMap.end( ) )
	 {
         toReturn.rwlock = lockMap.at( subBlockInfo ); 
         lockMapLock.unlock();

         DEBUG( "Found a lock for (%u, %u).\n",
               subBlockInfo.blockIndex, subBlockInfo.subBlockIndex );
	 }
      else
	{
        toReturn.rwlock = new threading::ReadWriteLock( );
        lockMap.insert( { subBlockInfo, toReturn.rwlock } );
        lockMapLock.unlock();

        DEBUG( "No lock found for block (%u, %u). Making a new one.\n",
               subBlockInfo.blockIndex, subBlockInfo.subBlockIndex );
	}

      DEBUG( "Getting a %s lock on it.\n", writing ? "write" : "read" );

      if ( writing )
         toReturn.rwlock->writeLock( );
      else
         toReturn.rwlock->readLock( );
      }

   DEBUG( "Lock acquired for (%u, %u).\n",
         subBlockInfo.blockIndex, subBlockInfo.subBlockIndex );

   assert( blockSize != 0 );

   blockMapLock.lock( );
   if ( blockMap.find( subBlockInfo.blockIndex ) != blockMap.end( ) )
      {
      auto pair = blockMap.at( subBlockInfo.blockIndex );
      toReturn.mmappedArea = pair.first + subBlockOffset;
      pair.second++;
      }
   else
      {
      char* mmappedArea = 
            ( char* )mmapWrapper( indexFD, blockSize, blockOffset );
      toReturn.mmappedArea = mmappedArea + subBlockOffset;
      blockMap.insert( { subBlockInfo.blockIndex, { mmappedArea, 1 } } );
      }
   blockMapLock.unlock( );

   return toReturn;
   }


void Postings::MunmapSubBlock( SubBlock subBlock )
   {
   assert( subBlock.mmappedArea != nullptr && blockSize != 0 );
   char* mmappedArea = subBlock.mmappedArea -
         subBlock.subBlockInfo.subBlockIndex * subBlock.subBlockInfo.subBlockSize;

   blockMapLock.lock( );
   auto pair = blockMap.at( subBlock.subBlockInfo.blockIndex );
   if ( --pair.second == 0 )
      msyncWrapper( mmappedArea, blockSize );
   blockMapLock.unlock( );

   subBlock.mmappedArea = nullptr;

   if ( subBlock.rwlock == nullptr )
      { 
      printf("Unlocking a lock that doesn't exist!\n");
      exit( 1 );
      }

   DEBUG( "Unlocking the %s lock on (%u, %u).\n",
         subBlock.writingLock ? "write" : "read",
         subBlock.subBlockInfo.blockIndex, subBlock.subBlockInfo.subBlockIndex );

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
