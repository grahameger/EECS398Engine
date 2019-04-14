#include "PersistentHashMap.h"
#include "Pair.h"
#include "vector.h"
#include "String.h"
#include "threading.h"
struct wordLocations{
   unsigned numWords;
   String word;
   Vector <unsigned long long> locations;
   wordLocations() = default;
   wordLocations(wordLocations&& toMove)
      :numWords(toMove.numWords), word(toMove.word), locations(toMove.locations){}
   };


class PriorityQueue{
public:
   PriorityQueue();
   //IT IS addWord()'s JOB TO DELETE THE wordLocations, they are passed by pointer, this destructor exists in case the program breaks
   ~PriorityQueue();
   void insert(String word, Vector<unsigned long long>* locationsVector);
   void pop();
   wordLocations* top();
   int size();
private:
   //maps to heap node struct
   PersistentHashMap<String, int> map;
   //holds dynamically allocated wordLocations
   Vector<wordLocations* > heap;
   int parentNode(int n);
   int rightNode(int n);
   int leftNode(int n);
   //for removing top node
   void down(int node);
   //for inserting node
   void up(int node);


};
