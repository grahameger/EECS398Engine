#ifndef INDEX_H
#define INDEX_H

// Index Class
// ###########
// Index is stored on disk as a file of distinctly allocated 8KB blocks.
//  (It is one flat file, but we access it in chunks of 8KB at a time.)
//  These blocks reserve the first 32 bits as a "pointer to next" or index of
//  the block following it. And the next is a UTF-8 like number (see next
//  paragraph) for number of words in the block. (Prevents you from reading
//  garbage).
//
// Words are stored as:
//    'w' 'o' 'r' 'd' '\0' tf index_num index_num ... index_num
//  Where the letters are stored in UTF-8 and tf & index_num is of a UTF-8 like 
//  type (Unary preamble of num bytes followed by a 0) to conserve space.
//
// So a block with the first 20 bytes:
//    00 00 03 90  0B 77 6F 72  64 00 03 01  66 83 AA 61  64 64 00 04 ...
// Would break up into the following:
//    912 11 word 3 1 102 938 add 4
//  Where 912 is the "pointer to next" or index of the next block after this,
//  11 is the number of words in this block, 'word' is the first word in this 
//  index and it appears 3 times at 1, 102, and 938. The next word is 'add' which 
//  appears 4 times and has index_nums to follow. Note that 912 is a regular 4 
//  byte integer, but 3, 1, 102, 938, and 4 are the UTF-8 like integers being 
//  represented by:
//    03, 01, 66, 83 AA, and 04
//  respectively.
//
// To add an index_num to an existing word, pull it's block into memory and add to
//  that word's index_num list. The re-caching mechanism will save it to the right
//  block.
// 
// To add a word to an index, pull the block of the last word before it and add 
//  the new word/index_num list (associated with the block). The re-caching mechanism
//  will save it to the right block.
//
// The first 1024 blocks in the index are a "hash-table" from word to block number.
//  These blocks start with a pointer to the next in the "hash-list" (collision
//  list for the "hash-table"). The next number is the number of words in THIS
//  block. Followed by the words and their blocks.
//
//

class Index 
   {
public:
   // For the parser
   void AddDocument( ParsedDocument* doc );

   // For queries
   // TODO: Decide what information queries need

private:


   };

#endif
