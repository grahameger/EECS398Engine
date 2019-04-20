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


// #####################
// Other Data Structures
// #####################

// SubBlock

struct SubBlock
   {
   bool uninitialized;
   char* mmappedArea;
   unsigned dataOffset;
   SubBlockInfo subBlockInfo;

   void SetNextPtr( unsigned blockIndex );
   bool operator!= ( const SubBlock& other ) const;
   bool operator== ( const SubBlock& other ) const;
   StringView ToStringView( );
   };


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
   StringView plStringView = plSubBlock.ToStringView( );

   // Create the posting list for this word
   PostingList postingList;
   if ( !plSubBlock.uninitialized )
      postingList = PostingList( plStringView );

   // Add each posting
   for ( unsigned i = 0; i < postings->size( ); i ++ )
      postingList.AddPosting( postings->at( i ) );

   #ifdef TEST
   printf("Added postings to %s posting list at SubBlock: (%u, %u) of size %u.\n", 
         plSubBlock.uninitialized ? "new" : "existing", 
         plSubBlock.subBlockInfo.blockIndex, plSubBlock.subBlockInfo.subBlockIndex, 
         plSubBlock.subBlockInfo.subBlockSize);
   printf("Posting list is now %u bytes.\n", postingList.GetByteSize( ));
   #endif

   // Update in place ( if possible ) and return
   if ( postingList.GetByteSize( ) <= plStringView.Size( ) )
      {
      #ifdef TEST
      printf("Putting posting list back in place\n");
      #endif

      postingList.UpdateInPlace( plStringView );
      return;
      }

   // Split new end posting into x blocks
   auto split = postingList.Split( blockSize - BlockDataOffset );

   #ifdef TEST
   printf("Putting posting list back into %lu block%s", split.size( ), 
         split.size( ) == 1 ? "" : "s" );
   #endif

   // Save the new split postingLists
   SaveSplitPostingList( plSubBlock, plStringView, split, word );
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
      SetOpen( sizeInfo );
      SetLastUsed( sizeInfo );
      }

   munmapWrapper( metaData.RawCString( ), metaData.Size( ) );
   }


void Postings::SaveSplitPostingList( SubBlock plSubBlock, StringView plStringView, 
      Vector< PostingList* >& split, const FixedLengthString& word )
   {
   unsigned blockIndexPtr = 0;

   for ( unsigned i = split.size( ); i > 0; i-- )
      {
      SubBlock newSubBlock;

      // This is a completely new block for the overflow data
      if ( i > 1 )
         newSubBlock = GetNewSubBlock( blockSize );

      // The old block wasn't full sized and needs to be upgraded
      else if ( plSubBlock.subBlockInfo.subBlockSize != blockSize )
         {
         newSubBlock = GetNewSubBlock( split[ i - 1 ]->GetByteSize( ) );
         subBlockIndex[ word ] = newSubBlock.subBlockInfo;
         wordIndex.insert( { newSubBlock.subBlockInfo, word } );
         }

      // The old block was a full sized block and just needs to be updated
      else
         {
         #ifdef TEST
         printf(", and the old block at (%u, %u) of size %u\n", 
               plSubBlock.subBlockInfo.blockIndex, 
               plSubBlock.subBlockInfo.subBlockIndex, 
               plSubBlock.subBlockInfo.subBlockSize);
         #endif

         split[ i - 1 ]->UpdateInPlace( plStringView );
         plSubBlock.SetNextPtr( blockIndexPtr );
         MunmapSubBlock( plSubBlock );
         return;
         }

      #ifdef TEST
      printf("%s at (%u, %u) of size %u", 
            i == split.size( ) ? "" : i == 1 ? ", and" : ",", 
            newSubBlock.subBlockInfo.blockIndex, 
            newSubBlock.subBlockInfo.subBlockIndex, 
            newSubBlock.subBlockInfo.subBlockSize);
      #endif

      // Set it's nextPtr
      if ( newSubBlock.subBlockInfo.subBlockSize == blockSize )
         newSubBlock.SetNextPtr( blockIndexPtr );

      // Fill in Posting List data
      StringView data( newSubBlock.mmappedArea + newSubBlock.dataOffset, 
            newSubBlock.subBlockInfo.subBlockSize - newSubBlock.dataOffset );
      split[ i - 1 ]->FullUpdate( data );

      blockIndexPtr = newSubBlock.subBlockInfo.blockIndex;
      MunmapSubBlock( newSubBlock );
      }

   #ifdef TEST
   printf("\n");
   #endif

   // Delete the outgrown subBlock
   if ( split.size( ) == 1 )
      DeleteSubBlock( plSubBlock );
   }


// TODO: PostingList that needs to look at next
Pair< unsigned, PostingList* > Postings::GetPostingList
      ( const FixedLengthString& word )
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

   return { nextPtr, postingList };
   }


Pair< unsigned, PostingList* > Postings::GetPostingList
      ( unsigned blockIndex )
   {
   // Grab that block
   SubBlock plSubBlock = GetSubBlock( { blockSize, blockIndex, 0 } );
   StringView plStringView = plSubBlock.ToStringView( );

   unsigned nextPtr = *( unsigned* )plSubBlock.mmappedArea;

   PostingList* postingList = new PostingList( plStringView );

   return { nextPtr, postingList };
   }


SubBlock Postings::GetPostingListSubBlock
      ( const FixedLengthString& word, bool endWanted )
   {
   auto wordIt = subBlockIndex.find( word );

   // Return the last block in the chain of blocks stored
   if ( wordIt != subBlockIndex.end( ) )
      return GetSubBlock( ( *wordIt ).second, endWanted );
   // Return a new block
   else
      {
      SubBlock subBlock = GetNewSubBlock( SmallestSubBlockSize( ) );
      subBlockIndex.insert( { word, subBlock.subBlockInfo } );
      wordIndex.insert( { subBlock.subBlockInfo, word } );
      return subBlock;
      }
   }


SubBlock Postings::GetNewSubBlock( unsigned minSize )
   {
   unsigned actualSize = blockSize;
   while ( actualSize / 2 >= minSize ) { actualSize /= 2; }

   SubBlockInfo subBlockInfo = GetOpenSubBlock( actualSize );

   if ( subBlockInfo.blockIndex == 0 )
      {
      subBlockInfo = { actualSize, nextBlockIndex, 0 };
      SetOpen( subBlockInfo );
      IncrementNumBlocks( );
      }
   IncrementOpenSubBlock( actualSize );

   SubBlock subBlock = MmapSubBlock( subBlockInfo );
   subBlock.uninitialized = true;

   return subBlock;
   }


SubBlock Postings::GetSubBlock( SubBlockInfo subBlockInfo, bool endWanted )
   {
   SubBlock subBlock = MmapSubBlock( subBlockInfo );

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
         subBlock = MmapSubBlock( subBlockInfo );
         }
      }

   return subBlock;
   }


void Postings::DeleteSubBlock( SubBlock subBlock )
   {
   assert( subBlock.subBlockInfo.subBlockSize != blockSize );

   // Get the last used sub block of this size
   SubBlockInfo lastUsedInfo = 
         GetLastUsedSubBlock( subBlock.subBlockInfo.subBlockSize );
   SubBlock lastUsedBlock = GetSubBlock( lastUsedInfo );

   if ( lastUsedBlock.subBlockInfo.blockIndex == 0 )
      {
      MunmapSubBlock( subBlock );
      return;
      }

   // This was not the last subBlock
   if ( lastUsedInfo != subBlock.subBlockInfo )
      {
      // Copy it into current subBlock
      memcpy( subBlock.mmappedArea, lastUsedBlock.mmappedArea, 
            lastUsedBlock.subBlockInfo.subBlockSize );
      // Fix hashmaps
      FixedLengthString word = wordIndex.at( lastUsedInfo );
      wordIndex.erase( lastUsedInfo );
      wordIndex.at( subBlock.subBlockInfo ) = word;
      subBlockIndex.at( word ) = subBlock.subBlockInfo;
      }

   // Set open to last used
   SetOpen( lastUsedInfo );
   // Decrement last used
   if ( lastUsedInfo.subBlockIndex == 0 )
      lastUsedInfo.blockIndex = 0;
   else
      lastUsedInfo.subBlockIndex--;
   SetLastUsed( lastUsedInfo );
   // Unmap the deleted block
   MunmapSubBlock( subBlock );
   }


SubBlockInfo Postings::GetOpenSubBlock( unsigned subBlockSize ) const
   {
   if ( subBlockSize == blockSize )
      return { subBlockSize, nextBlockIndex, 0 };

   // Size of each open/lastused entry
   unsigned offset = sizeof( unsigned ) * 2 + sizeof( unsigned char ) * 2;
   // Indexed based on subBlockSize
   offset *= blockSize / subBlockSize;
   // Plus the original offset from the other data
   offset += sizeof( unsigned ) * 2;

   SubBlockInfo returnInfo;

   returnInfo.subBlockSize = subBlockSize;
   returnInfo.blockIndex = metaData.GetInString< unsigned >( offset );
   offset += sizeof( unsigned );
   returnInfo.subBlockIndex = metaData.GetInString< unsigned char >( offset );

   return returnInfo;
   }


SubBlockInfo Postings::GetLastUsedSubBlock( unsigned subBlockSize ) const
   {
   // Should never be called for blockSize
   assert( subBlockSize != blockSize );

   // Size of each open/lastused entry
   unsigned offset = sizeof( unsigned ) * 2 + sizeof( unsigned char ) * 2;
   // Indexed based on subBlockSize
   offset *= blockSize / subBlockSize;
   // Plus the original offset from the other data
   offset += sizeof( unsigned ) * 3 + sizeof( unsigned char );

   SubBlockInfo returnInfo;

   returnInfo.subBlockSize = subBlockSize;
   returnInfo.blockIndex = metaData.GetInString< unsigned >( offset );
   offset += sizeof( unsigned );
   returnInfo.subBlockIndex = metaData.GetInString< unsigned char >( offset );

   return returnInfo;
   }


void Postings::IncrementOpenSubBlock( unsigned subBlockSize )
   {
   if ( subBlockSize == blockSize )
      return IncrementNumBlocks( );

   SubBlockInfo curOpen = GetOpenSubBlock( subBlockSize );
   SetLastUsed( curOpen );

   // Increment subBlockIndex
   ++curOpen.subBlockIndex %= blockSize / curOpen.subBlockSize;
   // If we spilled into a new block
   if ( curOpen.subBlockIndex == 0 )
      curOpen.blockIndex = 0;

   SetOpen( curOpen );
   }


inline void Postings::IncrementNumBlocks( )
   {
   metaData.SetInString< unsigned >( ++nextBlockIndex, sizeof( unsigned ) );
   }


void Postings::SetLastUsed( SubBlockInfo lastUsed )
   {
   // Size of each open/lastused entry
   unsigned offset = sizeof( unsigned ) * 2 + sizeof( unsigned char ) * 2;
   // Indexed based on subBlockSize
   offset *= blockSize / lastUsed.subBlockSize;
   // Plus the original offset from the other data
   offset += sizeof( unsigned ) * 3 + sizeof( unsigned char );

   // Set last used block for size subBlockSize
   metaData.SetInString< unsigned >( lastUsed.blockIndex, offset );
   offset += sizeof( unsigned );
   // Set last used subBlock for size subBlockSize
   metaData.SetInString< unsigned char >( lastUsed.subBlockIndex, offset );
   }


void Postings::SetOpen( SubBlockInfo open )
   {
   // Size of each open/lastused entry
   unsigned offset = sizeof( unsigned ) * 2 + sizeof( unsigned char ) * 2;
   // Indexed based on subBlockSize
   offset *= blockSize / open.subBlockSize;
   // Plus the original offset from the other data
   offset += sizeof( unsigned ) * 2;

   // Set open block for size subBlockSize
   metaData.SetInString< unsigned >( open.blockIndex, offset );
   offset += sizeof( unsigned );
   // Set open subBlock for size subBlockSize
   metaData.SetInString< unsigned char >( open.subBlockIndex, offset );
   }


SubBlock Postings::MmapSubBlock( SubBlockInfo subBlockInfo )
   {
   // BlockIndex * blockSize + subBlockIndex * subBlockSize
   unsigned blockOffset = subBlockInfo.blockIndex * blockSize;
   unsigned subBlockOffset = subBlockInfo.subBlockIndex * subBlockInfo.subBlockSize;

   unsigned dataOffset = 
         subBlockInfo.subBlockSize == blockSize ? BlockDataOffset : 0;

   SubBlock toReturn = { false, nullptr, dataOffset, subBlockInfo };
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
   }


unsigned Postings::SmallestSubBlockSize( ) const
   {
   unsigned numSizes = metaData.GetInString< unsigned >( 2 * sizeof( unsigned ) );
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
