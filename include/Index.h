#ifndef INDEX_H
#define INDEX_H

#include "StringView.h"
#include "PersistentHashMap.h"

class ParsedDocument;
class PostingListBlock;
class FixedLengthString;


struct SubBlockInfo
   {
   unsigned blockSize;
   unsigned blockIndex;
   unsigned char subBlockIndex;
   };


class Index 
   {
   public:
      Index( const char* filename, unsigned blockSize = DefaultBlockSize,
            unsigned numSizes = DefaultNumSizes );

   private:
      void CreateNewIndexFile( const char* filename, unsigned blockSize, 
            unsigned numSizes );
      PostingListBlock GetPostingListEnd( const FixedLengthString& word );

      StringView metaData;
      unsigned nextBlockIndex;
      unsigned blockSize;
      int indexFD;

      PersistentHashMap< unsigned, unsigned > subBlockIndex;

      static const unsigned DefaultBlockSize;
      static const unsigned DefaultNumSizes;

   };

#endif
