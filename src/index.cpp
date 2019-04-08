#include "index.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

	Index::Index(String filename):locks(1028) {
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
      for(int i = 0; i<1028; i++){
         locks[i] = ReadWriteLock();
      }
    }

	void Index:newDoc(string url){
		//add page end
		//add url to url list
		//put page metadata in url list

		//design of page end block:
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
      locks[pageEndBlock].writeLock();
      char* buf = new char[blockSize];
      readBlock(buf, pageEndBlock);
		//get newest url block
      int blockNum = followPointer(buf);

		int indexPointer;
		memcpy(&indexPointer, buf, sizeof(int));


      //need the location in utf8 form for the index
		String utf8Location = utf8(currentLocation);
		//read in the index
		PostingListIndex index = PostingListIndex(buf, blockSize-1-sizeof(int) ,indexPointer);
		//find offset and real location of most recently added page
		//index.findNewestPost();
		int delta = currentLocation - index.largestLocation()
		//convert delta to utf8 for saving
		String utf8Delta = utf8(delta);
      //add new delta this must be done before index.update
      memcpy(buf + index.nextOpenChar(), utf8Delta.CString(), utf8Delta.Size());
		//check if there is space to add page end
		index.update(utf8Location, index.nextOpenChar(), utf8Delta.Size());
		if(utf8Delta.Size() + index.nextOpenChar() > index.pointer()){
			//not enough space, need to move to next block
			char* pointerWriter[sizeof(int)];
			nextBlockLock.lock();
         int nextBlock = incrementNextEmptyBlock();
			nextBlockLock.unlock();
			//need to convert nextEmptyBlock to a cstring to write it
         memcpy(pointerWriter, &nextBlock, sizeof(int));
         //update block pointer, buf was fucked up by memcpying the delta when it didnt fit, so I use pointer writer
         writeLocation(pointerWriter, blockNum, blockSize - 1 - sizeof(int), sizeof(int));		
         //locks[blockNum] is write locked by followPointer()
         locks[blockNum].unlock();

         //create a new url block
         locks[nextBlock].writeLock();
         readBlock(buf, nextBlock);
         PostingListIndex newIndex = PostingListIndex(utf8Location, utf8Location.Size());
         //last byte in block is blockSize-1, first byte of blockPointer is that - sizeof(int), first block of index is that - index.size()
         indexPointer = newIndex.pointer();
         //update indexPointer
         memcpy(buf, &indexPointer, sizeof(int));
         //put in first delta 
         memcpy(buf + sizeof(int), utf8Delta.CString(), utf8Delta.Size());
         //put in index
         memcpy(buf + indexPointer, newIndex.string().CString(), newIndex.size());
         //save it
         writeBlock(buf, nextBlock);
         locks[nextBlock].unlock();
		}
		else{
         //update index pointer
         indexPointer = index.pointer();
         memcpy(buf, &indexPointer, sizeof(int));
         //update index
         memcpy(buf + indexPointer, Index.string().CString(), index.size());
			
			//save the block
         writeBlock(buf, blockNum);
         locks[blockNum].unlock();
		}
		currentLocation++;
		//this concludes the page end portion of newDoc()
		locks[urlBlock].writeLock();
      readBlock(buf, urlBlock);
      //get newest url block
		int blockNum = followPointer(buf);

		memcpy(&indexPointer, buf, sizeof(int)); 
		PostingListIndex urlIndex = PostingListIndex(buf, blockSize-1-sizeof(int) ,indexPointer);
		//is url passed as utf8?
		String utf8Url = urlAndDataToUtf8(url);
      //add new url
      memcpy(buf + urlIndex.nextOpenChar(), utf8Url.CString(), utf8Url.Size());
		//url fits in this block
		urlIndex.update(currentDocId, urlIndex.nextOpenChar(), utf8Url.Size());
		if(utf8Url.Size() + index.nextOpenChar() > urlIndex.pointer()){
			//not enough space in current block, must make a new one
			char* pointerWriter[sizeof(int)];
			nextBlockLock.lock();
         int nextBlock = incrementNextEmptyBlock();
			nextBlockLock.unlock();
			memcpy(pointerWriter, &nextBlock, sizeof(int));
			//write pointer to new block in current block
         writeLocation(pointerWriter, blockNum, blockSize - 1 - sizeof(int), sizeof(int));		
			locks[blockNum].unlock();


         locks[nextBlock].writeLock();
         readBlock(buf, nextBlock);
         PostingListIndex newIndex = PostingListIndex(currentDocId, utf8Url.Size());
         //update index pointer
         memcpy(buf, newIndex.pointer(), sizeof(int));
         //add new url
         memcpy(buf + sizeof(int), utf8Url.CString(), utf8Url.Size());
      	//update index
      	memcpy(buf + newIndex.pointer(), newIndex.string().CString(), newIndex.Size());
         writeBlock(buf, nextBlock);
		   locks[nextBlock].unlock();
		}
		else{
         //update index pointer
         memcpy(buf, urlIndex.pointer(), sizeof(int));
      	//update index
      	memcpy(buf + urlIndex.pointer(), urlIndex.string().CString(), urlIndex.Size());
         writeBlock(buf, blockNum);
		   locks[blockNum].unlock();
      }
		currentDocId++;
		delete[] buf;		
	}

void Index::addWord(String word, String wordData){
	int block = findWordBlock(word);



}

void Index::followPointer(char* buf, int blockNum){
   int pointer = 0;
   memcpy(&pointer, buf + blockSize - 5, sizeof(int));
   while( pointer != 0){
      locks[blockNum].unlock();
      blockNum = pointer;
      locks[blockNum].writeLock();
      readBlock(buf, blockNum);
      memcpy(&pointer, buf + blockSize - 5, sizeof(int));
   }
   return blockNum;
}

void Index::findWordBlock(String word){

}


void Index::incrementNextEmptyBlock(){
   if(nextEmptyBlock == numBlocks - 1){
		numBlocks *= 2;
		if(ftruncate(fd, numBlocks*blockSize) == -1){

		}
		
	}
   ReadWriteLock blockLock;
   locks.push_back(blockLock);
	//return nextEmptyBlock then increment
   return nextEmptyBlock++;
}

void Index::readBlock(char* buf, int blockNum){
   if(lseek(fd, blockNum*blockSize, SEEK_SET) == -1){

   }
   if(read(fd, buf, blockSize){

   }
}

void Index::writeBlock(char* buf, int blockNum){
   if(lseek(fd, blockNum*blockSize, SEEK_SET) == -1){

   }
   if(write(fd, buf, blockSize){

   }
}

void Index::readLocation(char* buf, int blockNum, int offset, int length){
   if(lseek(fd, blockNum*blockSize + offset, SEEK_SET) == -1){

   }
   if(read(fd, buf, length){

   }
}

void Index::writeLocation(char* buf, int blockNum, int offset, int length){
   if(lseek(fd, blockNum*blockSize + offset, SEEK_SET) == -1){

   }
   if(write(fd, buf, length){

   }
}

int PostingListIndex::pointer(){
   return blockSize - 1  - sizeof(int) - size;

}



