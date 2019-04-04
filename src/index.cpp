#include "index.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

	Index::Index(String filename){
		fd = open(filename.CString(), O_RDWR | O_CREAT);
		if(fd == -1){

		}
	//start with 1026 blocks, 0-1023 are dictionary
	//1024 is disk backed variables
	//1025 is page end block
	//1026 is list of urls
	//1027 is the first block for holding posting lists
		if(ftruncate(fd, numBlocks*blockSize) == -1){

		}
		currentBlocks.resize(numOfPostingSizes, 0);
		currentBlocks[0] = 1027;
    }

	void Index:newDoc(string url){
		//add page end
		//add url to url list
		//put page metadata in url list

		//design of page end block:
			//number of page end blocks, starts with 0, if greater than 0 dereference int at end of block X times to get to block with available space
			//offset to index
			//posts
			//empty space
			//index
			//pointer to next block, 0 if this block is not full

		//design of url list
			//offset to index
			//urls with metadata
			//empty space
			//index **this index will always include offset and docId of final url**
			//int that holds location of next block, 0 if this block is not full

		//step 1 find real location of previous doc end
		//step 2 delta = current location - previous doc end location
		//step 3 update index if necessary
		//step 4 current locaiton++
		//step 5 find location to place url+metadata
		//step 6 update url block index
		if(lseek(fd, pageEndBlock*blockSize, SEEK_SET) ==-1){

		}
		char* buf = new char[blockSize];
		if(read(fd, buf, blockSize) == -1){

		}
		int numPageEndBlocks;
		memcpy(&numPageEndBlocks, buf, sizeof(int));
		int indexPointer;
		memcpy(&indexPointer, buf + 4, sizeof(int));
		int pointer = 0;
		while(numPageEndBlocks > 0){
			memcpy(&pointer, buf + 8187, sizeof(int));
			if(lseek(fd, pointer*blockSize, SEEK_SET) ==-1){

			}
			if(read(fd, buf, blockSize) == -1){

			}
			numPageEndBlocks--;
			if(numPageEndBlocks == 0){
				//index pointer lives at char0 in all page end blocks othe than the first
				memcpy(&indexPointer, buf, sizeof(int));
			}
		}
		String utf8Location = utf8(currentLocation);
		//read in the index
		PostingListIndex index = PostingListIndex(buf, blockSize-1-sizeof(int) ,indexPointer);
		//find offset and real location of most recently added page
		index.findNewestPost();
		int delta = currentLocation - index.largestLocation()
		//convert delta to utf8 for saving
		String utf8Delta = utf8(delta);
		//check if there is space to add page end
		if(utf8Delta.Size() + index.nextOpenChar() > indexPointer){
			//not enough space, need to move to next block
			char* pointerWriter[sizeof(int)];
			//need to convert nextEmptyBlock to a cstring to write it
			memcpy(pointerWriter, &nextEmptyBlock, sizeof(int));
			if(lseek(fd, 8187, SEEK_CUR) ==-1){

			}
			//write pointer to new block in current block
			if(write(fd, pointerWriter, sizeof(int)) == -1){

			}
			if(lseek(fd, nextEmptyBlock*blockSize, SEEK_SET) ==-1){

			}
			if(read(fd, buf, blockSize) == -1){

			}
			incrementNextEmptyBlock();
			//TODO set index pointer, create index
			if(lseek(fd, 4, SEEK_CUR) ==-1){

			}
			if(write(fd, utf8Location.CString(), utf8Location.size());
		}
		else{
			char utf8DeltaChar = new char[utf8Delta.Size()];
			strcpy(buf + index.nextOpenChar(), utf8DeltaChar);
			//TODO update index
			
			//save the block
			if(pointer != 0){
				if(lseek(fd, pointer*blockSize, SEEK_SET) ==-1){

				}
			}
			else{
				if(lseek(fd, pageEndBlock*blockSize, SEEK_SET) ==-1){

				}
				
			}
			if(write(fd, buf, blockSize) == -1){

			}

		}
		currentLocation++;
		//this concludes the page end portion of newDoc()
		if(lseek(fd, blockSize*urlBlock, SEEK_SET) ==-1){
		
		}
		if(read(fd, buf, blockSize) == -1){

		}
		memcpy(&pointer, buf + 8187, sizeof(int));
		int blockNum = 0;
		while(pointer != 0){
			//follow the pointer until it is 0 signifiying a non full block
			blockNum = pointer;//so that the block num doesn't get written over a the end of the loop
			if(lseek(fd, blockSize*pointer, SEEK_SET) ==-1){
		
			}
			if(read(fd, buf, blockSize) == -1){

			}
			memcpy(&pointer, buf + 8187, sizeof(int));
			
		}	
		memcpy(&indexPointer, buf, sizeof(int)); 
		PostingListIndex urlIndex = PostingListIndex(buf, blockSize-1-sizeof(int) ,indexPointer);
		index.findNewestPost();		
		//is url passed as utf8?
		String utf8Url = urlAndDataToUtf8(url);
		if(utf8Url.Size() + index.nextOpenChar() > indexPointer){
			//not enough space in current block, must make a new one
			memcpy(pointerWriter, &nextEmptyBlock, sizeof(int));
			if(lseek(fd, 8187, SEEK_CUR) ==-1){

			}
			//write pointer to new block in current block
			if(write(fd, pointerWriter, sizeof(int)) == -1){

			}
			if(lseek(fd, nextEmptyBlock*blockSize, SEEK_SET) ==-1){

			}
			if(read(fd, buf, blockSize) == -1){

			}
			incrementNextEmptyBlock();
			//TODO update index
			if(lseek(fd, 4, SEEK_CUR) ==-1){

			}
			if(write(fd, utf8Location.CString(), utf8Location.size() == -1){

			}
		}
		else{
			//url fits in this block
			if(blockNum == 0){
				if(lseek(fd, urlBlock*blockSize + index.nextOpenChar();, SEEK_SET) ==-1){

				}
				if(write(fd, utf8Location.CString(), utf8Location.size() == -1){

				}
			}
			else{
				if(lseek(fd, blockNum*blockSize + index.nextOpenChar();, SEEK_SET) ==-1){

				}
				if(write(fd, utf8Location.CString(), utf8Location.size() == -1){

				}

			}
			//TODO update index
		}
		currentDocId++;
		delete[] buf;		
	}

void Index::addWord(String word, String wordData){
	int block = findWordBlock(word);



}


void Index::findWordBlock(String word){

}


void incrementNextEmptyBlock(){
	if(nextEmptyBlock == numBlocks - 1){
		numBlocks *= 2;
		if(ftruncate(fd, numBlocks*blockSize) == -1){

		}
		
	}
	nextEmptyBlock++;
}
