#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <mutex>
#include "crawler.h"
#include "PersistentBitVector.h"

int main(int argc, char *argv[]) {

	std::cout << sizeof(size_t) << '\n';

	std::vector<std::string> urls;
	//urls.reserve(1000000);
	std::ifstream start_list("../test/bigTest.url");
	std::string line;

	while (std::getline(start_list, line)) {
		urls.push_back(line);
	}
	search::Crawler crawler(urls);
}