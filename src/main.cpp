// Created by Graham Eger on 02/01/2019

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <mutex>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "crawler.h"
#include "PersistentHashMap.h"
#include "httpRequest.h"

// used to gracefully shut down and run all of our destructors
volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
    (void)_;
    keep_running = 0;
}

static const char startFile[] = "/data/crawl/dmoz/dmoz.base.urls";

static const std::vector<std::string> startFiles = {
	"/data/crawl/dmoz/dmoz.base.urls",
	"/data/crawl/reddit/reddit.dedupe.urls",
	"/data/crawl/hn/HNDump/deduped.hn.urls"
};

int main(int argc, char *argv[]) {

	// register our signal handler
	struct sigaction sa;
    memset( &sa, 0, sizeof(sa) );
    sa.sa_handler = sig_handler;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL);

	std::set<std::string> urls;
	std::ifstream start_list(startFile);
	std::string line;

	for (auto fileName : startFiles) {
		std::ifstream startFile(fileName);
		std::cout << "Reading file: " << fileName << std::endl;
		std::string line;
		while (std::getline(startFile, line)) {
			if (line != "") {
				HTTPRequest request(line);
				if (request.good()) {
					auto filename = request.filename();
					struct stat st;
					if (stat(filename.c_str(), &st) != 0) {
						urls.insert(line);
					}
				}
			}
		}
	}

	std::ifstream queue("queue.urls");
	while (std::getline(queue, line)) {
		if (line != "") {
			urls.insert(line);
		}
	}
	std::vector<std::string> urlsShuffled(urls.begin(), urls.end());
	// randomly shuffle the vector
	auto rng = std::default_random_engine {};
	std::shuffle(urlsShuffled.begin(), urlsShuffled.end(), rng);

	// fprintf(stdout, "Seedlist of %zd URLs imported from %s\n", urls.size(), startFile);
	fprintf(stdout, "Using %zd threads!\n", search::Crawler::NUM_CRAWLER_THREADS);
	search::Crawler crawler(urlsShuffled);
}
