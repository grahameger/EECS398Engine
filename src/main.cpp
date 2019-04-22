// Created by Graham Eger on 02/01/2019

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <algorithm>
#include <mutex>
//#include "index.h"
#include "String.h"
#include <signal.h>
#include <stdlib.h>
#include <unordered_set>
#include <stdio.h>
#include <getopt.h>
#include <thread>

#include "crawler.h"
#include "PersistentHashMap.h"
#include "httpRequest.h"
#include "http.h"
#include "Parser.hpp"
#include "threading.h"

#include <deque>
#include "PersistentHashMap.h"
#include "Parser.hpp"
#include "threading.h"
#include "index.h"


// used to gracefully shut down and run all of our destructors
volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
    (void)_;
    keep_running = 0;
}

static struct option longopts[] = {
   {"indexFilePrefix", required_argument, nullptr, 'i'},
   {"seedList", required_argument, nullptr, 's'},
   {"domainList", required_argument, nullptr, 'd'},
   {0, 0, 0, 0} //default
};
static const char startFile[] = "parsed.urls";
// static const char startFile[] = "/data/crawl/dmoz/dmoz.base.urls";

// static const std::vector<std::string> startFiles = {
//    "/data/crawl/dmoz/dmoz.base.urls",
//    "/data/crawl/reddit/reddit.dedupe.urls",
//    "/data/crawl/hn/HNDump/deduped.hn.urls"
// };

static const size_t NUM_PARSING_THREADS = 20;

FILE * fileOut;

int main(int argc, char *argv[]) {

   // register our signal handler
   struct sigaction sa;
    memset( &sa, 0, sizeof(sa) );
    sa.sa_handler = sig_handler;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL);

		std::string indexFilePrefix;
		std::string seedList;
		std::string domainList;
		int optionIndex = 0;
		int c;

	// 	// loop to process the command line options
	// 	while ((c = getopt_long(argc, argv, "i:s:", longopts, &optionIndex)) != -1) {
	// 		switch (c) {
	// 			case 'i':
	// 				indexFilePrefix = optarg;
	// 				break;
	// 			case 's':
	// 				seedList = optarg;
	// 				break;
	// 			case 'd':
	// 				domainList = optarg;
	// 			default:
	// 				fprintf(stderr, "Unknown option\n");
	// 				return 1;
	// 		}
	// 	}

	// FILE * fileOut = fopen("fullyparsed.urls", "w+");
	// if (!fileOut) {		
	// 	std::cerr << "couldn't open file" << std::endl;
	// 	exit(1);
	// }

	
	// open every file in the pages directory
	std::deque<std::string> files;
	std::deque<Doc_object> documents;
	threading::Mutex documentsMutex;
	threading::ConditionVariable documentsCv;
	DIR * dir;
	struct dirent * ent; 
	if ((dir = opendir("pages")) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			files.push_back(ent->d_name);
		}
	}

	fprintf(stdout, "starting to parse\n");

	std::thread threads[NUM_PARSING_THREADS];
	for (size_t i = 0; i < NUM_PARSING_THREADS; ++i) {
		threads[i] = std::thread(search::parseFiles, files, std::ref(documents), std::ref(documentsMutex), std::ref(documentsCv));
	}
	for (size_t i = 0; i < NUM_PARSING_THREADS; ++i) {
		threads[i].join();
	}
	fprintf(stdout, "done parsing\n");
	
	


	// Index index(&documents, &documentsMutex, &documentsCv);
  // wait until all the parsing is actually complete
	std::vector<std::string> seedListUrls;
	for (size_t i = 0; i < documents.size(); ++i) {
		for (size_t j = 0; i < documents[i].vector_of_link_anchor.size(); ++j) {
			seedListUrls.push_back(documents[i].vector_of_link_anchor[j].link_url.CString());
		}
	}
	std::ifstream seedListFile(startFile);
	std::string url;
	while (std::getline(seedListFile, url)) {
		seedListUrls.push_back(url);
	}
	auto rng = std::default_random_engine {};
	std::shuffle(seedListUrls.begin(), seedListUrls.end(), rng);
	fprintf(stdout, "Seedlist of %zd URLs imported from %s\n", seedListUrls.size(), startFile);
	fprintf(stdout, "Using %zd threads!\n", search::Crawler::NUM_CRAWLER_THREADS);
	search::Crawler crawler(seedListUrls);
}
