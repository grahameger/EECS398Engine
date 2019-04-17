#include "mmap.h"
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

// FixedLengthString

class FixedLengthString
   {
   public:
      FixedLengthString( const char* cstring );

      bool operator== ( const FixedLengthString& other ) const;

   private:
      static const unsigned MaxLength = 19;
      char characters[ MaxLength + 1 ];

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

   auto wordIt = subBlockIndex.find( 0 );

   if ( wordIt != subBlockIndex.end( ) )
      printf("butter found!\n");
   else
      {
      subBlockIndex.insert( Pair< unsigned, unsigned >( 0, 7 ) );
      assert( subBlockIndex.find( 0 ) != subBlockIndex.end( ) );
      printf("butter not found...\n");
      }
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


