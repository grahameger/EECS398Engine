#include <deque>
#include "threading.h"
#include "PriorityQueue.h"
#include "PersistentHashMap.h"
#include "Postings.h"
#include "vector.h"
#include "crawler.h"

// #include "String.h"
// #include "Pair.h"
// #include "StringView.h"
// #include "PostingList.h"
// #include "Parser.hpp"
// #include "hash_table.hpp"


class Index
   {
   public:
      Index( std::deque< Doc_object >* docQueue, threading::Mutex* queueLock, 
            threading::ConditionVariable* CV );
      void reader( );
      void writerDriver( );

   private:
   // STRUCTS
      struct urlMetadata
         {
         int length;
         int urlLength;
         int urlSlashes;
         int inLinks;
         int outLinks;
         char domain;
         unsigned importance;

         urlMetadata( ) : length( 0 ), urlLength( 0 ), urlSlashes( 0 ),
               inLinks( 0 ), outLinks( 0 ), domain( 'x' ), importance( 745000 )
            {}

         urlMetadata( int length, int urlLength, int urlSlashes, int inLinks, 
               int outLinks, char domain, unsigned importance )
               : length( length ), urlLength( urlLength ), urlSlashes( urlSlashes ),
               inLinks( inLinks ), outLinks( outLinks ), domain( domain ), 
               importance( importance )
            {}

         urlMetadata( const urlMetadata& u ) = default;
         };

   //FUNCTIONS

   // TODO: Max, is there a function you got rid of?
   //returns blocks that contains word's posting list
   //if posting list does not exist creates it, immediately updates blocks word index to hold this word

   //VARIABLES
      //true if priority queue is empty
      bool emptyQueue;
      //averageDocLength.bin file descriptor
      int fd;
      //the id of the doc we are currently parsing
      int currentDocId;
      //reader() waits until its docId = currentWriteDocId to push to the pQueue
      int currentWriteDocId;
      //total length of all the docs we've parsed
      unsigned long long totalDocLength;
      //current absolute location on the web
      unsigned long long currentLocation;

      //priority queue of words to be added to the index
      PriorityQueue queue;
      //queue of doc objects from parser
      std::deque< Doc_object >* documentQueue;
      //TODO: FIGURE OUT HOW TO STORE URLS
      PersistentHashMap< unsigned long long, FixedLengthURL > urlMap;
      PersistentHashMap< FixedLengthURL, urlMetadata > metaMap;

   //THREADING
      Vector< pthread_t > readThreads;
      Vector< pthread_t > writeThreads;
      threading::ConditionVariable* dequeCV;
      threading::ConditionVariable queueReadCV;
      threading::ConditionVariable queueWriteCV;
      threading::Mutex pQueueLock;
      threading::Mutex currentLocationMutex;
      threading::Mutex currentWriteDocIdMutex;
      threading::Mutex* documentQueueLock;

};
