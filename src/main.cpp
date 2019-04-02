#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <mutex>
#include "crawler.h"
#include "PersistentHashMap.h"

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
	const ssize_t maxVal = 1000000;
	PersistentHashMap<ssize_t, ssize_t> map(String("test"));
	Pair<ssize_t, ssize_t> insert;
	for (ssize_t i = 0; i < maxVal; i++) {
		insert.first = i;
		insert.second = i * -1;
		map.insert(insert);
		if (i % 50000 == 0) {
			std::cout << i << std::endl;
		}
	}
	// run it back
	for (ssize_t i = 0; i < maxVal; i++) {
		std::cout << map.at(i) << '\n';
	}
}