#include "PriorityQueue.h"
#include "stdexcept"

PriorityQueue::PriorityQueue( )
   { }

PriorityQueue::~PriorityQueue( )
   {
   for( unsigned i = 0; i < heap.size( ); i++ )
      {
      delete heap[ i ];
      }
   }


void PriorityQueue::insert( String word, Vector< unsigned long long >* locationsVector )
   {
   size_t index;
   wordLocations* locations;
   auto unIt = unallowedWords.find(word);
   //check if current word is unallowed
   if(unIt != unallowedWords.end())
   {
      for(unsigned i = 0; i < locationsVector->size(); i++)
      {
         unIt->second.push_back( locationsVector->operator[ ]( i ) );
      }
      return;
   }

   auto mapIt = map.find( word );
   if ( mapIt != map.end( ) )
      {
      index = mapIt->second;
      locations = heap[ index ];
      }
   else
      {
      //map does not contain word
      index = heap.size();
      map[ word ] = index;
      locations = new wordLocations( );
      locations->word = std::move( word );
      heap.push_back( locations );
      }

   /*
   try
      {
      index = map.at( word );
      locations = heap[ index ];
      }
   catch( std::out_of_range& oor )
      {
      //map does not contain word
      index = heap.size();
      map[ word ] = index;
      locations = new wordLocations( );
      locations->word = std::move( word );
      heap.push_back( locations );
      }
   */

   //update *locations using the vector passed
   for( unsigned i = 0; i < locationsVector->size( ); i++ )
      {
      locations->locations.push_back( locationsVector->operator[ ]( i ) );
      locations->numWords++;
      }

   up( index );
   }


void PriorityQueue::pop( )
   {
   if( heap.size( ) == 0 )
      return;

   //erase word from map
   map.erase( heap[ 0 ]->word );

   //move last index to front update map, down()
   heap[ 0 ] = heap[ heap.size() - 1 ];
   map[ heap[ 0 ]->word ] = 0;
   heap.pop_back( );
   down( 0 );
   }

wordLocations* PriorityQueue::top( )
   {
   //the top word becomes unallowed until allow(word) is called
   unallowedWords[ heap[ 0 ]->word ];
   return heap[ 0 ];
   }

size_t PriorityQueue::size( )
   {
   return heap.size( );
   }

void PriorityQueue::allow(String word)
   {
   auto it = unallowedWords.find(word);
   if(it != unallowedWords.end())
      {
         //must delete unallowedWords[word] before inserting
         Vector<unsigned long long> locations = it->second;
         unallowedWords.erase(word);
	 if ( locations.size( ) > 0 )
	    insert(word, &locations);
      }
   }

size_t PriorityQueue::parentNode( size_t n )
   {
   return ( n - 1 ) / 2;
   }

size_t PriorityQueue::rightNode( size_t n )
   {
   return ( 2 * n + 2 );
   }

size_t PriorityQueue::leftNode( size_t n )
   {
   return ( 2 * n + 1 );
   }

void PriorityQueue::down( size_t node )
   {
   if( node > heap.size( ) )
      return;

   size_t leftIndex = leftNode( node );
   size_t rightIndex = rightNode( node );
   
   size_t largestIndex = node;

   if( leftIndex < heap.size( ) && 
         heap[ leftIndex ]->numWords > heap[ largestIndex ]->numWords )
      {
      largestIndex = leftIndex;
      }
   if( rightIndex < heap.size( ) && 
         heap[ rightIndex ]->numWords > heap[ largestIndex ]->numWords )
      {
      largestIndex = rightIndex;
      }

   if( largestIndex != node )
      {
      wordLocations* temp = heap[ largestIndex ];
      heap[ largestIndex ] = heap[ node ];
      heap[ node ] = temp;

      map[ heap[ largestIndex ]->word ] = largestIndex;
      map[ heap[ node ]->word ] = node;
      down( largestIndex );
      }
   }

void PriorityQueue::up( size_t node )
   {
   if( node < heap.size( ) && node != 0 )
      {
      size_t parentIndex = parentNode( node );
      if( heap[ node ]->numWords > heap[ parentIndex ]->numWords )
         {
         wordLocations* temp = heap[ parentIndex ];
         heap[ parentIndex ] = heap[ node ];
         heap[ node ] = temp;
         //fix the map
         map[ heap[ node ]->word ] = node;         
         map[ heap[ parentIndex ]->word ] = parentIndex;
         up( parentIndex );
         }
      }
   }
