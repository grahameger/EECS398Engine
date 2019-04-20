#include "String.h"
#include "vector.h"
#include "threading.h"
#include "PersistentHashMap.h"
#include "Pair.h"
#include "StringView.h"
#include "PostingList.h"
#include "Postings.h"
#include "PriorityQueue.h"
#include "Parser.hpp"
#include "hash_table.hpp"
#include <list>


class Index{
public:
	Index(String filename, std::list<Doc_object>* docQueue, threading::Mutex* queueLock);
	~Index();
   void reader();

private:
	//FUNCTIONS

	//returns blocks that contains word's posting list
	//if posting list does not exist creates it, immediately updates blocks word index to hold this word
	void writerDriver(void* notNeeded);
	//VARIABLES
   PriorityQueue queue;
   //queue of doc objects from parser
   std::list<Doc_object>* documentQueue;
   struct urlMetadata{
	   int length;
      int urlLength;
      int urlSlashes;
      int inLinks;
      int outLinks;
      urlMetadata()
         :length(0), urlLength(0), urlSlashes(0), inLinks(0), outLinks(0){}
      urlMetadata(int length, int urlLength, int urlSlashes, int inLinks, int outLinks)
         :length(length), urlLength(urlLength), urlSlashes(urlSlashes), inLinks(inLinks), outLinks(outLinks){}
      urlMetadata(const urlMetadata& u)
         :length(u.length), urlLength(u.urlLength), urlSlashes(u.urlSlashes), inLinks(u.inLinks), outLinks(u.outLinks){}
   };
	//map will prob be moved to scheduler
	//FIGURE OUT HOW TO STORE URLS
   PersistentHashMap<unsigned long long, FixedLengthString> urlMap;
   PersistentHashMap<fixedLengthString, urlMetadata> metaMap;
   //current absolute location on the web
	unsigned long long currentLocation;
	//true if priority queue is empty
   bool emptyQueue;
   int currentDocId;
   //reader() waits until its docId = currentWriteDocId to push to the pQueue
   int currentWriteDocId;

   //THREADING
   Vector<pthread_t> readThreads;
   Vector<pthread_t> writeThreads;
   threading::ConditionVariable queueReadCV;
   threading::ConditionVariable queueWriteCV;
   threading::Mutex pQueueLock;
   threading::Mutex currentLocationMutex;
   threading::Mutex currentWriteDocIdMutex;
   threading::Mutex* documentQueueLock;
};
