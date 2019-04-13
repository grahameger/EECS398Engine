#include "String.h"
#include "vector.h"
#include "threading.h"
#include "PersistentHashMap.h"
#include "Pair.h"
#include "StringView.h"
#include "PostingList.h"


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
	void threadDriver(void* notNeeded);
	void reader();
   int incrementNextEmptyBlock();
   //returns the block and sub block that the next posting list of size postingBlockSizes[index] will be moved into
   //CALLS TO THIS MUST BE LOCKED WITH currentBlocksLock
   locationPair getCurrentBlock(unsigned index);
   //reading and writing functions
   void readBlock(char* buf, int blockNum);
   void readLocation(char* buf, int blockNum, int offset, int length);
   void writeBlock(char* buf, int blockNum);
   void writeLocation(char* buf, int blockNum, int offset, int length);
   void writeLocation(const char* buf, int blockNum, int offset, int length);
   //input is char* of a block with int at the end which is a pointer, output is the same char* containing block with pointer = 0
   int followPointer(char* buf, int blockNum);
   //returns the index of smallest posting list block that string of length byte size will fit in
   unsigned int smallesFit(unsigned int byteSize);
	//VARIABLES
   priority_queue<wordLocations> queue;
   struct wordLocations{
      unsigned numWords;
      String word;
      vector <unsigned long long> locations;
      wordLocations(wordLocations&& toMove)
         :numWords(toMove.numWords), word(toMove.word), locations(toMove.locations){}
   };
   
	struct locationPair{
      int blockNum;
      int offset;
      locationPair():blockNum(0), offset(0){}
      locationPair(const locationPair& copy):blockNum(copy.blockNum), offset(copy.offset){}
      locationPair(int blockNumber, int off):blockNum(blockNumber), offset(off){}
   };
	//map will prob be moved to scheduler
   PersistentHashMap<String, locationPair> map;
	const int blockSize;
	int parserFd;
	int fd;
	int numOfPostingSizes;
	Vector<locationPair> currentBlocks;
	Vector<unsigned int> postingBlockSizes;
	const int pageEndBlock;
	const int urlBlock;
	long currentLocation;//increment at end of addWord and newDoc
	//increment next empty block, if out of blocks make the file bigger
	int nextEmptyBlock;
	int numBlocks;
	int currentDocId;
   bool doneReadingIn;
   //THREADING
   vector<pthread_t> threads;
   Vector<threading::ReadWriteLock*> locks;
	threading::Mutex queueLock;
   threading::Mutex nextBlockLock;
   threading::Mutex currentBlocksLock;
   threading::Mutex mapLock;
   threading::Mutex currentLocationMutex;
   threading::Mutex currentDocIdMutex;
   struct urlMetadata{
		//not sure what we need here
	};
};
/*
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
   unsigned long long largestLocation();
   //returns the first unused charecter in this posting list
   unsigned int nextOpenChar();
   int size();
   //updates the index with new post at offset in block, location in internet, size of post
   void update(unsigned long long location, int offset, int length);
   String string();
   //returns location of index pointer. URL and page end blocks have an int after the index, set intOffset true
   int pointer(int blockSize);
private:
   String index; 
   int indexSize;
};

class PostingList{
public:
   PostingList(int fd, int startOffset, int listLength);
   //if the updated posting List fits in its block it returns 1, otherwise 0
   int update(unsigned long long location);
   String string();
   //returns the block size of this plist
   int length();
private:
   String pList;
   StringView posts;
   PostingListIndex index;
   int listLength;
};
*/

