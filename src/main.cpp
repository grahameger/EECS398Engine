#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <mutex>
#include "crawler.h"
#include "PersistentHashMap.h"

static const char startFile[] = "/home/coder/EECS398Engine/test/parsed.urls";

int main(int argc, char *argv[]) {
	std::vector<std::string> urls;
	std::ifstream start_list(startFile);
	std::string line;

	while (std::getline(start_list, line)) {
		urls.push_back(line);
	}
	fprintf(stdout, "Seedlist of %zd URLs imported from %s\n", urls.size(), startFile);
	fprintf(stdout, "Using %zd threads!\n", search::Crawler::NUM_CRAWLER_THREADS);
	search::Crawler crawler(urls);

	// PersistentBitVector v("testVector");
	// v.set(0, true);
	// v.set(15, true);
	// assert(v.at(0) && v.at(15));
	// bool fifteen = v.at(15);
	// bool zero = v.at(0);
	// bool ten = v.at(10);
	// assert(fifteen && zero && !ten);

	// const ssize_t maxVal = 100000;
	// PersistentHashMap<ssize_t, ssize_t> map(String("test"));
	// Pair<ssize_t, ssize_t> insert;
	// for (ssize_t i = 0; i < maxVal; i++) {
	// 	insert.first = i;
	// 	insert.second = i * -1;
	// 	map.insert(insert);
	// 	if ( i % 10000 == 0 ) {
	// 		std::cout << i << std::endl;
	// 	}
	// }
	// // run it back
	// for (ssize_t i = 0; i < maxVal; i++) {
	// 	auto datum = map.at(i);
	// 	assert(datum == -1 * i);
	// }
}