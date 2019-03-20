#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <mutex>
#include "http.hpp"
#include "thread_queue.hpp"
#include "semaphore.hpp"

int main(int argc, char *argv[]) {
	std::vector<std::string> urls;
	urls.reserve(1000000);
	std::ifstream start_list("../test/parsed.urls");
	std::string line;

	while (std::getline(start_list, line)) {
		urls.push_back(line);
	}

	search::Crawler crawler(urls);
}