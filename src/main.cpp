#include <iostream>
<<<<<<< HEAD
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <mutex>
//#include "index.h"
#include "String.h"
#include "PriorityQueue.h"
int main(int argc, char *argv[]) {

	// std::cout << sizeof(size_t) << '\n';

	// std::vector<std::string> urls;
	// urls.reserve(1000000);
	// std::ifstream start_list("../test/bigTest.url");
	// std::string line;

	// while (std::getline(start_list, line)) {
	// 	urls.push_back(line);
	// }
	// search::Crawler crawler(urls);
   /*
	PersistentBitVector v("testVector");
	v.set(0, true);
	v.set(15, true);
	assert(v.at(0) && v.at(15));
	bool fifteen = v.at(15);
	bool zero = v.at(0);
	bool ten = v.at(10);
	assert(fifteen && zero && !ten);

	const ssize_t maxVal = 100000;
	PersistentHashMap<ssize_t, ssize_t> map(String("test"));
	Pair<ssize_t, ssize_t> insert;
	for (ssize_t i = 0; i < maxVal; i++) {
		insert.first = i;
		insert.second = i * -1;
		map.insert(insert);
		if ( i % 10000 == 0 ) {
			std::cout << i << std::endl;
		}
	}
	// run it back
	for (ssize_t i = 0; i < maxVal; i++) {
		auto datum = map.at(i);
		assert(datum == -1 * i);
	}
   */
   PriorityQueue pq; 

   Vector<unsigned long long> locations;
   for(unsigned i = 0; i < 10; i++){
      locations.push_back(i);
   }
   pq.insert(String("first"), &locations);
   locations.push_back(100);
   pq.insert(String("second"), &locations);
   Vector<unsigned long long> locations2;
   for(unsigned i = 0; i < 10; i++){
      locations2.push_back(i);
   }
   pq.insert(String("first"), &locations2);
   locations.push_back(100);
   pq.insert(String("third"), &locations);
   locations.push_back(100);
   pq.insert(String("4"), &locations);
   

   std::cout<< pq.top()->word.CString()<<std::endl <<pq.top()->numWords<<std::endl;
   delete pq.top();
   pq.pop();
   std::cout<< pq.top()->word.CString()<<std::endl <<pq.top()->numWords<<std::endl;
   delete pq.top();
   pq.pop();
   std::cout<< pq.top()->word.CString()<<std::endl <<pq.top()->numWords<<std::endl;
   delete pq.top();
   pq.pop();
   std::cout<<"end"<<std::endl;
   /*
   String file = "index.bin";
   // Index index(file);
   int fd = open(file.CString(), O_RDWR|O_CREAT);

   PostingList pl(fd, 0, 64);
   pl.update(7);
   pl.update(10);
   pl.update(66000);
   */
}
