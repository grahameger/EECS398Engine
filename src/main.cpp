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

// used to gracefully shut down and run all of our destructors
volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
    (void)_;
    keep_running = 0;
}

static const char startFile[] = "/data/crawl/seedlist.url";

int main(int argc, char *argv[]) {

	// register our signal handler
	struct sigaction sa;
    memset( &sa, 0, sizeof(sa) );
    sa.sa_handler = sig_handler;
    sigfillset(&sa.sa_mask);
    sigaction(SIGINT,&sa,NULL);


	std::vector<std::string> urls;
	std::ifstream start_list(startFile);
	std::string line;

	while (std::getline(start_list, line)) {
		if (line != "") {
			urls.push_back(line);
		}
	}
	urls.push_back("http://dmoztools.net/");
	// urls.push_back("http://soshesawildflowerxo.tumblr.com/post/173338544891/deep-talks-are-my-favorite-if-you-can-connect#_=_");

	// fprintf(stdout, "Seedlist of %zd URLs imported from %s\n", urls.size(), startFile);
	fprintf(stdout, "Using %zd threads!\n", search::Crawler::NUM_CRAWLER_THREADS);
	search::Crawler crawler(urls);
}
