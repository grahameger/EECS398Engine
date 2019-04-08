#include "String.h"
#include "vector.h"
#include "threading.h"



class Index{
public:
	Index(String filename);
	void addWord(String word, wordData);
	void newDoc(String url);

private:
	//FUNCTIONS

	//returns blocks that contains word's posting list
	//if posting list does not exist creates it
	int findWordBlock(String word);
	int hash(String word);
	int hash2(String word);
	int incrementNextEmptyBlock();

   //reading and writing functions
   void readBlock(char* buf, int blockNum);
   void readLocaiton(char* buf, int blockNum, int offset, int length);
   void writeBlock(char* buf, int blockNum);
   void writeLocation(char* buf, int blockNum, int offset, int length);
   //input is char* of a block with int at the end which is a pointer, output is the same char* containing block with pointer = 0
   void followPointer(char* buf, int blockNum);
	//VARIABLES
	vector<int> currentBlocks;
	const int blockSize = 8192;
	
	int fd;
	int numOfPostingSizes = ;
	vector<int> postingBlockSizes = {  };
	const int pageEndBlock = 1025;
	const int urlBlock = 1026;
	long currentLocation = 0;//increment at end of addWord and newDoc
	//increment next empty block, if out of blocks make the file bigger
	int nextEmptyBlock = 1028;
	int numBlocks = 10000;
	int currentDocId = 0;
   
   //THREADING
   vector<ReadWriteLock> locks;
	mutex nextBlockLock;
   struct urlMetadata{
		//not sure what we need here
	};
};

class Post{

}

class PostingList{

}

class WordIndex{
   //this is the index that maps posting lists to block offsets
public:
   WordIndex(char* buf, int startOffset, int endOffset);
   int findWord(String word);
   void update(String word, int offset);
private:
   String index; 


}

class PostingListIndex{
   //this is the index contained in a posting list that contains offsets to posts
public:
   //for reading in a posting list index
   PostingListIndex(char* buf, int startOffset, int endOffset);
   //for creating a new posting list index
   PostingListIndex(String location, int length);
   
   //returns the largest location in this index
   int largestLocation();
   //returns the first unused charecter in this posting list
   int nextOpenChar();
   int size();
   //updates the index with new post at offset in block, location in internet, size of post
   void update(String location, String offset, int length);
   String string();
   //returns location of index pointer URL and page end blocks
   int pointer();
private:
   String index; 
   const int blockSize = 8192
};
