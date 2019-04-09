#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <mutex>
#include "crawler.h"
#include "PersistentHashMap.h"

static const char startFile[] = "/home/coder/EECS398Engine/test/so.url";

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
}
