#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <mutex>
<<<<<<< HEAD
#include "crawler.h"
#include "PersistentHashMap.h"

// static const char startFile[] = "/data/crawl/seedlist.url";
=======
//#include "index.h"
#include "String.h"
#include "PriorityQueue.h"
int main(int argc, char *argv[]) {
>>>>>>> a5dda953de39ce29de7fb69c5ab847bdc5fb35b2

int main(int argc, char *argv[]) {

	std::vector<std::string> urls;
	// std::ifstream start_list(startFile);
	// std::string line;

	// while (std::getline(start_list, line)) {
	// 	if (line != "") {
	// 		urls.push_back(line);
	// 	}
	// }
<<<<<<< HEAD
	urls.push_back("http://literatureismyutopia.tumblr.com/post/130165038543/celebrate-banned-books-week#_=_");

	// fprintf(stdout, "Seedlist of %zd URLs imported from %s\n", urls.size(), startFile);
	fprintf(stdout, "Using %zd threads!\n", search::Crawler::NUM_CRAWLER_THREADS);
	search::Crawler crawler(urls);
=======
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
>>>>>>> a5dda953de39ce29de7fb69c5ab847bdc5fb35b2
}
