#include "index.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "Utf8Uint.h"

Index::Index(String filename)
   :blockSize(10000 + sizeof(int)), pageEndBlock(1), urlBlock(2), currentLocation(0), nextEmptyBlock(3), numBlocks(10000), currentDocId(0), map("table"), currentBlocks(numOfPostingSizes), threads(10), doneReadingIn(false) {


   parserFd = open(parserFile, O_RDONLY);
   fd = open(filename.CString(), O_RDWR | O_CREAT);
   if(fd == -1){

	}
	//start with 1026 blocks, 0-1023 are dictionary
	//0 is disk backed variables
	//1 is page end block
	//2 is list of urls
	//3 is the first block for holding posting lists
	if(ftruncate(fd, numBlocks*blockSize) == -1){

	}
	currentBlocks[0] = locationPair(3,0);
   for(int i = 0; i<1000; i++){
      threading::ReadWriteLock* lock = new threading::ReadWriteLock;
      locks.push_back(lock);
   }
   for(unsigned i = 0; i < threads[i].size(); i++){
      pthread_create(&threads[i], NULL, &index::threadDriver, (void*)0);
   }

   //read in
   reader();
}

Index::~Index(){
   for(int i = 0 ; i < locks.size(); i++){
      delete locks[i];
   }
}

	void Index::newDoc(String url){
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
      locks[pageEndBlock]->writeLock();
      char* buf = new char[blockSize];
      readBlock(buf, pageEndBlock);
		//get newest url block
      int blockNum = followPointer(buf, pageEndBlock);

		int indexPointer;
		memcpy(&indexPointer, buf, sizeof(int));


      //need the location in utf8 form for the index
		//String utf8Location = utf8(currentLocation);
		
      //read in the index, if pointer==0 create a new index
      PostingListIndex index = indexPointer == 0 ? PostingListIndex(currentLocation) : PostingListIndex(buf + indexPointer, blockSize - indexPointer);
		//find offset and real location of most recently added page
		//index.findNewestPost();
		int delta = currentLocation - index.largestLocation();
		//convert delta to utf8 for saving
		String utf8Delta("holder"); //= utf8(delta);
      //add new delta this must be done before index.update
      memcpy(buf + index.nextOpenChar(), utf8Delta.CString(), utf8Delta.Size());
		//check if there is space to add page end
		index.update(currentLocation, index.nextOpenChar(), utf8Delta.Size());
		if(utf8Delta.Size() + index.nextOpenChar() > index.pointer(blockSize)){
			//not enough space, need to move to next block
			char* pointerWriter[sizeof(int)];
			nextBlockLock.lock();
         int nextBlock = incrementNextEmptyBlock();
			nextBlockLock.unlock();
			//need to convert nextEmptyBlock to a cstring to write it
         memcpy(pointerWriter, &nextBlock, sizeof(int));
         //update block pointer, buf was fucked up by memcpying the delta when it didnt fit, so I use pointer writer
         writeLocation((char*)pointerWriter, blockNum, blockSize - 1 - sizeof(int), sizeof(int));		
         //locks[blockNum] is write locked by followPointer()
         locks[blockNum]->unlock();

         //create a new url block
         locks[nextBlock]->writeLock();
         readBlock(buf, nextBlock);
         PostingListIndex newIndex = PostingListIndex(currentLocation);
         //last byte in block is blockSize-1, first byte of blockPointer is that - sizeof(int), first block of index is that - index.size()
         indexPointer = newIndex.pointer(blockSize);
         //update indexPointer
         memcpy(buf, &indexPointer, sizeof(int));
         //put in first delta 
         memcpy(buf + sizeof(int), utf8Delta.CString(), utf8Delta.Size());
         //put in index
         memcpy(buf + indexPointer, newIndex.string().CString(), newIndex.size());
         //save it
         writeBlock(buf, nextBlock);
         locks[nextBlock]->unlock();
		}
		else{
         //update index pointer
         indexPointer = index.pointer(blockSize);
         memcpy(buf, &indexPointer, sizeof(int));
         //update index
         memcpy(buf + indexPointer, index.string().CString(), index.size());
			
			//save the block
         writeBlock(buf, blockNum);
         locks[blockNum]->unlock();
		}
		currentLocation++;
		//this concludes the page end portion of newDoc()
		locks[urlBlock]->writeLock();
      readBlock(buf, urlBlock);
      //get newest url block
		blockNum = followPointer(buf, urlBlock);

		memcpy(&indexPointer, buf, sizeof(int)); 
		//is url passed as utf8?
		String utf8Url("holder");// = urlAndDataToUtf8(url);
		PostingListIndex urlIndex = indexPointer == 0 ? PostingListIndex(currentDocId, utf8Url) : PostingListIndex(buf + indexPointer, blockSize-indexPointer);
      //add new url
      memcpy(buf + urlIndex.nextOpenChar(), utf8Url.CString(), utf8Url.Size());
		//url fits in this block
		urlIndex.update(currentDocId, urlIndex.nextOpenChar(), utf8Url.Size());
		if(utf8Url.Size() + index.nextOpenChar() > urlIndex.pointer(blockSize)){
			//not enough space in current block, must make a new one
			char* pointerWriter[sizeof(int)];
			nextBlockLock.lock();
         int nextBlock = incrementNextEmptyBlock();
			nextBlockLock.unlock();
			memcpy(pointerWriter, &nextBlock, sizeof(int));
			//write pointer to new block in current block
         writeLocation((char*)pointerWriter, blockNum, blockSize - 1 - sizeof(int), sizeof(int));		
			locks[blockNum]->unlock();


         locks[nextBlock]->writeLock();
         readBlock(buf, nextBlock);
         PostingListIndex newIndex = PostingListIndex(currentDocId, utf8Url);
         //update index pointer
         indexPointer = urlIndex.pointer(blockSize);
         memcpy(buf, &indexPointer, sizeof(int));
         //add new url
         memcpy(buf + sizeof(int), utf8Url.CString(), utf8Url.Size());
      	//update index
      	memcpy(buf + indexPointer, newIndex.string().CString(), newIndex.size());
         writeBlock(buf, nextBlock);
		   locks[nextBlock]->unlock();
		}
		else{
         //already added url
         //update index pointer
         indexPointer = urlIndex.pointer(blockSize);
         memcpy(buf, &indexPointer, sizeof(int));
      	//update index
      	memcpy(buf + indexPointer , urlIndex.string().CString(), urlIndex.size());
         writeBlock(buf, blockNum);
		   locks[blockNum]->unlock();
      }
		currentDocId++;
		delete[] buf;		
	}

void threadDriver(void* notNeeded){
   //put them in a priority queue that holds wordLocations, sorted by numWords
   //pop value from priority queue to addWord
   wordLocations* locations;
   while(!doneReadingIn){
      queueLock.lock();
      locations = &(queue.top());
      addWord(locations);
      //unlocked in addWord
   }
   
}

void reader(){
   //could this be threaded?
   //    would have to make sure thread driver only happens when all document older than the newest currently being read are completely read.
   //    reader must add to queue in correct order, has to wait for readers of old docs to finish if necessary

}

void Index::addWord(wordLocations* locationsToMove, int queueIndex){
   //adds a word to the index
   //
   //design of posting list block
   //cation,
   //    int block size
   //    int index pointer
   //    utf8 posting lists
   //    empty space
   //    utf8 word index
   //
   //step 1 find block that contains word's posting list
   //step 2 read that block and index
   //step 3 if the word already has a posting list attempt to add the delta to the posting list
   //    check if there is space in the posting list
   //    if there is space put the delta in, update the posting list index, update the index pointer, save the block
   //    if there is not space the posting list needs to be migrated to a block with bigger posting list
   //       find the block with next largest posting list size
   //       put current posting list in it, update word index and index pointer in new block
   //       add delta and update posting list index and index pointer
   //       attempt to migrate a posting list from current block of that posting list size to block that posting lsit was removed from
   //       if a posting list that fits is found make sure to update word index and index pointer
   //if the word does not have a posting list attempt to create one
   //    check if there is space
   //    if there is space update the word index and index pointer, create posting list
   //    if there is not space change current block of that size to next empty block
   //       initaite block with block size at location 0, create word's posting list in first posting list location
   //       update index and index pointer
   //
   //
   
   //set being used in function that passes locations for concurrency

   wordLocations locations(std::move(*locationsToMove));
   //now that locaitons is out of queue we remove the entry
   queue.remove(queueIndex);
   queueLock.unlock();
   ScheduleBlock sb = Scheduler::GetPostingList(locations->word);//write version?
   PostingList postingList(sb.pl);
   unsigned int beforeSizeIndex = smallestFit(postingList.GetByteSize());
   for(unsigned i = 0; i < locations->numWords; i++){
      postingList.AddPosting(locations->locations[i]);
   }
   if(smallestFit(postingList.GetByteSize()) - beforeSizeIndex == 0){
      //it fits
      postingList.UpdateInPlace();
   }
   else{
      //doesn't fit
      vector <PostingList> split = PostingList::Split(blockSize);
      unsigned blockPointer = 0;
      for(i = split.size() - 1; i > 0; i++){
         blockPointer = Scheduler::GetBlock(split[i], blockPointer);
      }
      sb.sb.UpgradeBlock(split[0], blockPointer);
      
   }

/*
	locationPair pair = map[word];
   char* buf = new char[blockSize];
   if(pair.blockNum == 0){
      //word not in map
      currentBlocksLock.lock();
	   //make sure we didnt get data raced
      if(pair.blockNum == 0){
         //posting list doesn't exist, must be created
         pair = locationPair(currentBlocks[0]);
         //copy constructor changes mapped struct value too
         nextPostingListBlock(postingBlockSizes[0]);
      }
      currentBlocksLock.unlock();
      //save block size
      writeLocation((char*)&postingBlockSizes[0], pair.blockNum, 0, sizeof(int));
   }
   locks[pair.blockNum]->writeLock();
   readBlock(buf, pair.blockNum);
   int listSize;
   memcpy(&listSize, buf, sizeof(int));
   int offset = pair.offset + sizeof(int);
   PostingList pList = PostingList(fd, pair.blockNum*blockSize + offset, listSize);
   if(pList.update(word) == 1){
      //fits
      writeLocation(pList.string().CString(), pair.blockNum, offset, listSize);

   }
   else{
      //doesnt fit
      currentBlocksLock.lock();

      currentBlocksLock.unlock();
   }

   locks[pair.blockNum]->unlock();


   int blockNum, offset;
   memcpy(&blockNum, pair, sizeof(int));
   memcpy(&offset, pair + sizeof(int), sizeof(int));
   char* buf = new char[blockSize];   
   locks[blockNum].writeLock();
   readBlock(buf, blockNum);
   int listSize;
   memcpy(&listSize, buf, sizeof(int));
   //memcpy(buf + sizeof(int), &indexPointer, sizeof(int));
   //WordIndex index = indexPointer == 0 ? WordIndex(word) : WordIndex(buf, blocksize-1, indexPointer);
   //int offset = index.findWord(word);
   //posting list found
   //check if posting list can be expanded, if not move it to a bigger list size
   if(pList.update(currentLocation) == 0){
      //updated posting list fits in its block
      writeLocation(pList.String().CString(), blockNum, offset, listSize);
      locks[blockNum].unlock();
      //EZ
   }
   else{
      //find a posting list to move into list block that is being left
      locks[blockNum].unlock();
      currentBlocksLock.lock();
      for(int i = 0; i < numOfPostingSizes; i++){
         if(postingBlockSizes[i] == listSize){
            locks[currentBlocks[i]].writeLock();
         }
      }
      currentBlocksLock.unlock();
      
      //posting list needs to be moved to new block
      int* pair2 = nextBiggerPostingListBlock(listSize, word);


      memcpy(&blockNum, pair2, sizeof(int));
      memcpy(&offset, pair2 + sizeof(int), sizeof(int));
      writeLocation(pList.String().CString(), blockNum, offset, pList.length());
   }*/
}

int Index::incrementNextEmptyBlock(){
   if(nextEmptyBlock == numBlocks - 1){
		numBlocks *= 2;
		if(ftruncate(fd, numBlocks*blockSize) == -1){

		}
		for(int i = 0; i < numBlocks / 2; i++){
         threading::ReadWriteLock* lock = new threading::ReadWriteLock;
         locks.push_back(lock);
      }
	}
	//return nextEmptyBlock then increment
   return nextEmptyBlock++;
}

void Index::readBlock(char* buf, int blockNum){
   if(lseek(fd, blockNum*blockSize, SEEK_SET) == -1){

   }
   if(read(fd, buf, blockSize)){

   }
}

void Index::writeBlock(char* buf, int blockNum){
   if(lseek(fd, blockNum*blockSize, SEEK_SET) == -1){

   }
   if(write(fd, buf, blockSize)){

   }
}

void Index::readLocation(char* buf, int blockNum, int offset, int length){
   if(lseek(fd, blockNum*blockSize + offset, SEEK_SET) == -1){

   }
   if(read(fd, buf, length)){

   }
}

void Index::writeLocation(char* buf, int blockNum, int offset, int length){
   if(lseek(fd, blockNum*blockSize + offset, SEEK_SET) == -1){

   }
   if(write(fd, buf, length)){

   }
}

void Index::writeLocation(const char* buf, int blockNum, int offset, int length){
   if(lseek(fd, blockNum*blockSize + offset, SEEK_SET) == -1){

   }
   if(write(fd, buf, length)){

   }
}

int Index::followPointer(char* buf, int blockNum){
   int pointer = 0;
   memcpy(&pointer, buf + blockSize - 5, sizeof(int));
   while( pointer != 0){
      locks[blockNum]->unlock();
      blockNum = pointer;
      locks[blockNum]->writeLock();
      readBlock(buf, blockNum);
      memcpy(&pointer, buf + blockSize - 5, sizeof(int));
   }
   return blockNum;
}

unsigned int smallestFit(unsigned int byteSize){
   for(unsigned i = 0; i < postingBlockSizes.size(); i++){
      if(byteSize >= postingBlockSizes[i]) return i;
   }
}

/*
WordIndex::WordIndex(char* buf, int startOffset, int endOffset){

}

WordIndex::WordIndex(String Word){

}

int WordInex::findWord(String word){
   //returns offset to start of this words posting list

}

void WordIndex::update(String word, int offset){

}

PostingList::PostingList(int fd, int startOffset, int listLength)
   : posts(nullptr,0), listLength(listLength) {
   char* mapped =(char*)mmapWrapper(fd, listLength, startOffset);
   int indexPointer = *((int*)mapped);
   index = indexPointer == 0 ? PostingListIndex() : PostingListIndex(mapped + indexPointer, listLength);
   int postPointer = index.nextOpenChar();
   posts = StringView(mapped + sizeof(int), postPointer - sizeof(int));
}

int PostingList::update(unsigned long long location){
   unsigned int offset = index.nextOpenChar();
   unsigned long long largestLocation = index.largestLocation();
   unsigned long long delta  = location - largestLocation;

   Utf8Uint utf8(location);
   OutputByteStream obs;
   obs << utf8;
   StringView utf8Delta = obs.GetString();

   index.update(location, offset, utf8Delta.Size());
   int indexPointer = index.pointer(listLength);

   if(offset + utf8Delta.Size() > indexPointer){
      //need to grow the posting list
      listLength *= 2;
      char* buf = new char[listLength];
      memcpy(buf, pList.CString(), offset);
      indexPointer = index.pointer(listLength);
      memcpy(buf, &indexPointer, sizeof(int));
      memcpy(buf + offset, utf8Delta.GetCString(), utf8Delta.Size());
      memcpy(buf + indexPointer, index.string().CString(), index.size());
      pList = String(std::move((char*)buf), listLength);//might need to change string constructor
      return 0;
   }

   //add on the index and update the index pointer
   char* buf = new char[listLength];
   memcpy(buf, pList.CString(), offset);
   memcpy(buf + offset, utf8Delta.GetCString(), utf8Delta.Size());
   memcpy(buf + indexPointer, index.string().CString(), index.size());
   pList = String(std::move((const char*)buf), listLength);//might need to change string constructor
   return 1;
}



int PostingList::length(){
   return listLength;
}

PostingListIndex::PostingListIndex()
   :indexSize(3*sizeof(int) + sizeof(unsigned long long int)) {
   char* newString = new char[ indexSize ];
   *((int*)newString) = sizeof(int);
   index = String(std::move(newString), indexSize);
}

PostingListIndex::PostingListIndex(char* buf, int length)
   :indexSize(length), index(buf, length) {

}

PostingListIndex::PostingListIndex(int location){

}

PostingListIndex::PostingListIndex(int location, String url){

}

unsigned long long PostingListIndex::largestLocation(){
   return *(unsigned long long*)(index.CString() + sizeof(unsigned int));
}

unsigned int PostingListIndex::nextOpenChar(){
   return *(unsigned int*)index.CString();
}



int PostingListIndex::size(){
   return index.Size();
}

void PostingListIndex::update(unsigned long long location, int offset, int length){
   unsigned int pairPointer = *(unsigned int*)(index.CString() + 2*sizeof(unsigned int) + sizeof(unsigned long long));
   
   // TODO: Better
   InputByteStream ibs( String( index.CString() + pairPointer, 24 ) );
   Utf8Uint pairOffset, pairLocation;
   ibs >> pairOffset >> pairLocation;
   unsigned long long longOffset = (unsigned long long) offset;
   if(location >> 16 > pairLocation.GetValue() >> 16){
      //making a new pair
      OutputByteStream obs;
      obs << Utf8Uint(longOffset) << Utf8Uint(location);
      index += String(obs.GetString().GetCString());
      pairPointer += obs.GetString().Size();
      *(unsigned int*)(index.CString() + 2*sizeof(unsigned int) + sizeof(unsigned long long)) = pairPointer;
   }
   *(unsigned int*)index.CString() = offset + length;
   *(unsigned long long*)(index.CString() + sizeof(unsigned int)) = location;
}

String PostingListIndex::string(){
   return index;
}
//intOffset = true if there is an int afte rthe index in the positng list
int PostingListIndex::pointer(int blockSize){
   return blockSize - indexSize;

}

*/
