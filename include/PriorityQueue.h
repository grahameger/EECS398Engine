#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEU_H

#include <unordered_map>
#include "vector.h"
#include "String.h"
// #include "hash.h"
#include "Postings.h"
// #include "Pair.h"
// #include "threading.h"


struct wordLocations
   {
   unsigned numWords;
   FixedLengthString word;
   Vector < unsigned long long > locations;
   };


class PriorityQueue
   {
   public:
      PriorityQueue( );
      //IT IS addWord()'s JOB TO DELETE THE wordLocations, they are passed by posize_ter,
      //this destructor exists in case the program breaks
      ~PriorityQueue( );

      void insert( FixedLengthString word, Vector< unsigned long long >* locationsVector );
      void pop( );
      wordLocations* top( );
      size_t size( );
			void allow(FixedLengthString word);
   private:
      //maps to heap node struct
      std::unordered_map< FixedLengthString, size_t, hash::Hash< FixedLengthString > > map;

      std::unordered_map< FixedLengthString, Vector<unsigned long long>, hash::Hash< FixedLengthString > > unallowedWords;
      //holds dynamically allocated wordLocations
      Vector< wordLocations* > heap;
      size_t parentNode( size_t n );
      size_t rightNode( size_t n );
      size_t leftNode( size_t n );
      //for removing top node
      void down( size_t node );
      //for inserting node
      void up( size_t node );

   };

#endif
