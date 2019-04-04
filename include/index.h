#include "String.h"
#include "vector.h"




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
	void incrementNextEmptyBlock();

	//VARIABLES
	vector<int> currentBlocks;
	int fd;
	int blockSize = 8192;
	int numOfPostingSizes = ;
	vector<int> postingBlockSizes = {  };
	int pageEndBlock = 1025;
	int urlBlock = 1026;
	long currentLocation = 0;//increment at end of addWord and newDoc
	//increment next empty block, if out of blocks make the file bigger
	int nextEmptyBlock = 1028;
	int numBlocks = 10000;
	int currentDocId = 0;
	struct urlMetadata{
		//not sure what we need here
	};
};
