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
	for (ssize_t i = 0; i < maxVal; i++) {
		map.insert(Pair(i, i * -1));
	}
	// run it back
	for (auto &i : map) {
		std::cout << '{' << i.first << ", " << i.second << "}\n";
	}
}