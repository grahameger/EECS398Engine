#include "String.h"
#include "vector.h"
#include "threading.h"
#include "PersistentHashMap.h"
#include "Pair.h"
#include "StringView.h"
class Index{
public:
	Index(String filename);
	~Index();
   void addWord(String word,String wordData);
	void newDoc(String url);

private:
	//FUNCTIONS

	//returns blocks that contains word's posting list
	//if posting list does not exist creates it, immediately updates blocks word index to hold this word
	int* findWordBlock(String word);
	int hash(String word);
	int hash2(String word);
	int incrementNextEmptyBlock();
   //changes currentBlocks to the next location for listSize
   int nextPostingListBlock(int listSize);
   //reading and writing functions
   void readBlock(char* buf, int blockNum);
   void readLocation(char* buf, int blockNum, int offset, int length);
   void writeBlock(char* buf, int blockNum);
   void writeLocation(char* buf, int blockNum, int offset, int length);
   void writeLocation(const char* buf, int blockNum, int offset, int length);
   //input is char* of a block with int at the end which is a pointer, output is the same char* containing block with pointer = 0
   int followPointer(char* buf, int blockNum);
	//VARIABLES
	struct locationPair{
      int blockNum;
      int offset;
      locationPair():blockNum(0), offset(0){}
      locationPair(const locationPair& copy):blockNum(copy.blockNum), offset(copy.offset){}
      locationPair(int blockNumber, int off):blockNum(blockNumber), offset(off){}
   };
	PersistentHashMap<String, locationPair> map;
	const int blockSize;
	
	int fd;
	int numOfPostingSizes;
	Vector<locationPair> currentBlocks;
	Vector<int> postingBlockSizes;
	const int pageEndBlock;
	const int urlBlock;
	long currentLocation;//increment at end of addWord and newDoc
	//increment next empty block, if out of blocks make the file bigger
	int nextEmptyBlock;
	int numBlocks;
	int currentDocId;
   
   //THREADING
   Vector<threading::ReadWriteLock*> locks;
	threading::Mutex nextBlockLock;
   threading::Mutex currentBlocksLock;
   threading::Mutex mapLock;
   threading::Mutex currentLocationMutex;
   threading::Mutex currentDocIdMutex;
   struct urlMetadata{
		//not sure what we need here
	};
};

class PostingListIndex{
   //this is the index contained in a posting list that contains offsets to posts
public:
   //default
   PostingListIndex();
   //for reading in a posting list index
   PostingListIndex(char* buf, int length);
   //for creating a new posting list index,
   PostingListIndex(int location);
   //url page holds strings not locations, needs current docId
   PostingListIndex(int location, String url);
   //returns the largest location in this index
   int largestLocation();
   //returns the first unused charecter in this posting list
   int nextOpenChar();
   int size();
   //updates the index with new post at offset in block, location in internet, size of post
   void update(int location, int offset, int length);
   String string();
   //returns location of index pointer. URL and page end blocks have an int after the index, set intOffset true
   int pointer(int blockSize, bool intOffset);
private:
   String index; 
   int indexSize;
};

class PostingList{
public:
   PostingList(int fd, int startOffset, int listLength);
   //if the updated posting List fits in its block it returns 1, otherwise 0
   int update(unsigned long long int location);
   String string();
   //returns the block size of this plist
   int length();
private:
   StringView posts;
   PostingListIndex index;
   int listLength;
};
