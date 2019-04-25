#include "index.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "Utf8Uint.h"
#include <deque>
#include "hash_table.hpp"


void* readerWrapper( void* index )
   {
   static_cast< Index* >( index )->reader( );
   return nullptr;
   }

void* writerWrapper( void* index )
   {
   static_cast< Index* >( index )->writerDriver( );
   return nullptr;
   }


Index::Index( std::deque< Doc_object >* docQueue, threading::Mutex* queueLock, 
      threading::ConditionVariable* fullCV, threading::ConditionVariable* emptyCV) 
      : emptyQueue( false ), currentDocId( 0 ), currentWriteDocId( 0 ), 
      totalDocLength( 0 ), currentLocation( 1 ), documentQueue( docQueue ), 
      urlMap( "urlTable" ), metaMap( "metaTable" ), dequeCV( fullCV ), 
      queueWriteCV( emptyCV ), documentQueueLock( queueLock )
   {
   fd = open( "averageDocLength.bin", O_RDWR | O_CREAT, S_IRWXU );
   ftruncate( fd, sizeof( unsigned long long ) );

   //read in
   for( unsigned i = 0; i < 5; i++ )
      pthread_create( &readThreads[ i ], NULL, &readerWrapper, this );

   for( unsigned i = 0; i < 20; i++ )
      pthread_create( &writeThreads[ i ], NULL, &writerWrapper, this );

   pthread_join( readThreads[ 0 ], nullptr );

   //reader(docQueue);
}

void Index::writerDriver( )
   {
   //put them in a priority queue that holds wordLocations, sorted by numWords
   //pop value from priority queue to addWord
   wordLocations* locations;
   while( true )
      {
      pQueueLock.lock( );

      while( queue.size( ) == 0 )
         {
         emptyQueue = true;
         queueReadCV.wait( pQueueLock );
         }

      locations = queue.top( );

      //now that locaitons is out of queue we remove the entry
      queue.pop( );
      pQueueLock.unlock( );

      //pass fixed length word and location vector to AddPostings
      Postings* postings = Postings::GetPostings( );
      postings->AddPostings( FixedLengthString( locations->word.CString( ) ), 
            &locations->locations );

      pQueueLock.lock( );
      queue.allow( locations->word);
      pQueueLock.unlock( );
      delete locations;
      }
   }


void Index::reader( )
   {
   //could this be threaded?
   //    would have to make sure thread driver only happens when all document older than the newest currently being read are completely read.
   //    reader must add to queue in correct order, has to wait for readers of old docs to finish if necessary
   while( true )
      {
      documentQueueLock->lock( );
      while( documentQueue->empty( ) )
         {
	 queueWriteCV->wait( *documentQueueLock );
         }

      Doc_object doc = documentQueue->front( );
      documentQueue->pop_front( );

      unsigned long long startLocation = currentLocation;
      int docSize = doc.Words.size( ) + doc.url.size( );

      //docSize += all the anchor text document lengths we're adding
      for( unsigned i = 0; i < doc.vector_of_link_anchor.size( ); i++ )
         {
         docSize += doc.vector_of_link_anchor[ i ].anchor_words.size( );
         //empty anchor tags are not documents
         if(doc.vector_of_link_anchor[i].anchor_words.empty()) docSize--;
         }
      //every doc end is itarconv: No such file or directory own location, 1 for regular doc + 1 for each anchor text
      //docSize += # of endDocs
      docSize += doc.vector_of_link_anchor.size( ) + 1;

      currentLocation += docSize;
      int docId = currentDocId;
      currentDocId++;

      std::cout << "Popped: " << doc.doc_url.CString( ) << std::endl;

      //can't read in next doc until current location and currentDocId are updated
      documentQueueLock->unlock( );
      dequeCV->signal( );

      hash_table< Vector< unsigned long long > > localMap;
      //@-anchor
      //#-title
      //$-url
      //*-body

      //pass urls and doc ends to newDoc somehow, probably a queue of url, docEnd pairs
      //parse into word, vector<ull>location pairs
      for( unsigned i = 0; i < doc.url.size(); i++ )
         {
         String urlRegular( doc.url[ i ] );
         String urlDecorated = String( "$" ) + urlRegular;
         // Add to word posting list
         localMap[ urlRegular ]->push_back( startLocation );
         // Add to decorated word posting list
         localMap[ urlDecorated ]->push_back( startLocation );
         startLocation++;
         }
      for( unsigned i = 0; i < doc.Words.size( ); i++ ){
         //proabbly going to need a map here
         //if statement for where in the doc this occured
         if( doc.Words[ i ].type == 't' )
            {
            String titleRegular( doc.Words[ i ].word );
            String titleDecorated = String( "#" ) + titleRegular;
            // Add to word posting list
            localMap[ titleRegular ]->push_back( startLocation );
            // Add to decorated word posting list
            localMap[ titleDecorated ]->push_back( startLocation );
            startLocation++;
            }
         else
            {
            String bodyRegular( doc.Words[ i ].word );
            String bodyDecorated = String( "*" ) + doc.Words[ i ].word;
            // Add to word posting list
            localMap[ bodyRegular ]->push_back( startLocation );
            // Add to decorated word posting list
            localMap[ bodyDecorated ]->push_back( startLocation );
            startLocation++;
            }
      }
      //docEnd
      localMap[ String("") ]->push_back( startLocation++ );

      FixedLengthURL fixedLengthUrl( doc.doc_url.CString( ) );
      urlMap[ startLocation ] = fixedLengthUrl;

      //parse anchor texts
      for( unsigned i = 0; i < doc.vector_of_link_anchor.size( ); i++ )
         {
         //don't push empty anchor tag to doc end map, don't give it a doc end location
      	 if(doc.vector_of_link_anchor[i].anchor_words.empty()) continue;
			   for( unsigned j = 0; j < doc.vector_of_link_anchor[ i ].anchor_words.size( ); j++)
            {
            //probably gonna need to use the same map here
            String anchorRegular( doc.vector_of_link_anchor[ i ].anchor_words[ j ].word );
            String anchorDecorated = String( "@" ) + anchorRegular;
            // Add to word posting list
            localMap[ anchorRegular ]->push_back( startLocation );
            // Add to decorated word posting list
            localMap[ anchorDecorated ]->push_back( startLocation );
            startLocation++;
            }
         //docEnd map here
         localMap[ String("") ]->push_back( startLocation );

         FixedLengthURL fixedLengthUrl( doc.vector_of_link_anchor[ i ].link_url.CString( ) );
         urlMap[ startLocation++ ] = fixedLengthUrl;
         }

      FixedLengthURL fixedDocUrl( doc.doc_url.CString( ) );

      int inLinks = metaMap[ fixedDocUrl ].inLinks;
      urlMetadata metadata( doc.Words.size( ), doc.doc_url.Size( ), doc.num_slash_in_url,
            inLinks, doc.vector_of_link_anchor.size( ), doc.domain_type, doc.domain_rank );

      // Doc_object parsed, time to add it's words to the index priority queue
      // Can't add this doc until all before it have been added
      currentWriteDocIdMutex.lock( );
      while( currentWriteDocId != docId )
         {
         queueWriteDocIdCV.wait( currentWriteDocIdMutex );
         }
      currentWriteDocIdMutex.unlock( );

      // It's time to add this doc
      pQueueLock.lock( );

      //url -> metadata
      metaMap[ fixedDocUrl ] = metadata;
      for( unsigned i = 0; i < doc.vector_of_link_anchor.size( ); i++ )
         {
         FixedLengthURL anchorPointsTo( doc.vector_of_link_anchor[ i ].link_url.CString( ) );
         metaMap[ anchorPointsTo ].inLinks++;
         }

      // Update AverageDocLength.bin
      totalDocLength += doc.Words.size( );
      unsigned averageDocLength = totalDocLength / ( docId + 1 );
      lseek( fd, 0, SEEK_SET );
      write( fd, &averageDocLength, sizeof( unsigned ) );

      //iterate through map and insert into pQueue
      for( unsigned i = 0; i < localMap.numBuckets; i++ ){
         auto it = localMap.array[ i ].begin( );
         while( it != localMap.array[ i ].end( ))
            {
            //add word vector<ull>location pair to priority queue
            queue.insert( ( *it ).key, ( *it ).offset );
            it++;
            }
      }

      pQueueLock.unlock();

      // Notify write threads there is something in the pQueue
      if( emptyQueue )
         {
         emptyQueue = false;
         queueReadCV.broadcast( );
         }

      // Increment currentWriteDocId, then broadcast to anyone waiting
      currentWriteDocIdMutex.lock();
      currentWriteDocId++;
      currentWriteDocIdMutex.unlock();
      queueWriteDocIdCV.broadcast();

   }
}
