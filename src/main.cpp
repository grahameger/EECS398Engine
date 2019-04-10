#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <mutex>
#include "index.h"
#include "String.h"

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
   String file = "index.bin";
   // Index index(file);
   int fd = fopen(file, O_RDWR);

   PostingList pl(fd, 0, 64);
   pl.update(7);
   pl.update(10);
   pl.update(66000);
}
