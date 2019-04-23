#include <unordered_map>
#include "vector.h"
#include "String.h"
#include "hash.h"

// #include "Pair.h"
// #include "threading.h"


struct wordLocations
   {
   unsigned numWords;
   String word;
   Vector < unsigned long long > locations;
   };


class PriorityQueue
   {
   public:
      PriorityQueue( );
      //IT IS addWord()'s JOB TO DELETE THE wordLocations, they are passed by posize_ter,
      //this destructor exists in case the program breaks
      ~PriorityQueue( );

      void insert( String word, Vector< unsigned long long >* locationsVector );
      void pop( );
      wordLocations* top( );
      size_t size( );

   private:
      //maps to heap node struct
      std::unordered_map< String, size_t > map;

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
