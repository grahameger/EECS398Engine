#include "PriorityQueue.h"
#include "stdexcept"

PriorityQueue::PriorityQueue()
   :map(String("pQueue.map")){}

PriorityQueue::~PriorityQueue(){
   for(unsigned i = 0; i < heap.size(); i++){
      delete heap[i];
   }
}

void PriorityQueue::insert(String word, Vector<unsigned long long>* locationsVector){
   int index;
   wordLocations* locations;
   try{
      index = map.at(word);
      locations = heap[index];
   }
   catch(std::out_of_range& oor){
      //map does not contain word
      index = heap.size();
      map[word] = index;
      locations = new wordLocations;
      locations->word = word;
      heap.push_back(locations);
   }
   //update *locations using the vector passed
   for(unsigned i = 0; i < locationsVector->size(); i++){
      locations->locations.push_back(locationsVector->operator[](i));
      locations->numWords++;
   }
   up(index);
}

void PriorityQueue::pop(){
   if(heap.size() == 0){
      return;
   }
   //erase word from map
   map.erase(heap[0]->word);
   //move last index to front update map, down()
   heap[0] = heap[heap.size() - 1];
   map[heap[0]->word] = 0;
   heap.pop_back();
   down(0);
}

wordLocations* PriorityQueue::top(){
   return heap[0];
}

int PriorityQueue::size(){
   return heap.size();
}

int PriorityQueue::parentNode(int n){
   return(n - 1)/2;
}

int PriorityQueue::rightNode(int n){
   return(2 * n + 2);
}

int PriorityQueue::leftNode(int n){
   return(2 * n + 1);
}

void PriorityQueue::down(int node){
   if(node > heap.size()){
      return;
   }

   int leftIndex = leftNode(node);
   int rightIndex = rightNode(node);
   
   int largestIndex = node;

   if(leftIndex < heap.size() && heap[leftIndex]->numWords > heap[largestIndex]->numWords){
      largestIndex = leftIndex;
   }
   if(rightIndex < heap.size() && heap[rightIndex]->numWords > heap[largestIndex]->numWords){
      largestIndex = rightIndex;
   }
   if(largestIndex != node){
      wordLocations* temp = heap[largestIndex];
      heap[largestIndex] = heap[node];
      heap[node] = temp;

      map[heap[largestIndex]->word] = largestIndex;
      map[heap[node]->word] = node;
      down(largestIndex);
   }

}

void PriorityQueue::up(int node){
   if(node < heap.size() && node != 0){
      int parentIndex= parentNode(node);
      if(heap[node]->numWords > heap[parentIndex]->numWords){
         wordLocations* temp = heap[parentIndex];
         heap[parentIndex] = heap[node];
         heap[node] = temp;
         //fix the map
         map[heap[node]->word] = node;         
         map[heap[parentIndex]->word] = parentIndex;
         up(parentIndex);
      }
   }
};
