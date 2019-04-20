// Created by Graham Eger on 02/01/2019

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <mutex>
//#include "index.h"
#include "String.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <list>
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

static const char startFile[] = "/data/crawl/seedlist.url";

int main(int argc, char *argv[]) {
/*
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
*/


   std::list<Doc_object> docList;
   threading::Mutex queueLock;
   
   Index index(String("index.bin"), &docList, &queueLock);
   std::cout<<"here"<<std::endl;
   Doc_object d;
   Vector<Index_object> anchor;
   d.doc_url = String("github.com");
   d.num_slash_in_url = 1;
   d.Links.push_back(String("abc.com"));
   Index_object i;
   i.word = String("a");
   i.type = String("title");
   i.position = 0;
   d.Words.push_back(i);
   i.word = String("b");
   i.type = String("body");
   i.position = 1;
   d.Words.push_back(i);
   i.word = String("c");
   i.type = String("body");
   i.position = 2;
   d.Words.push_back(i);
   i.word = String("co");
   i.type = String("body");
   i.position = 3;
   d.Words.push_back(i);
   i.word = String("iabc");
   i.type = String("anchor");
   i.position = 4;
   anchor.push_back(i);
   d.Words.push_back(i);
   i.word = String("pows");
   i.type = String("anchor");
   i.position = 5;
   d.Words.push_back(i);
   anchor.push_back(i);
   d.anchor_words.push_back(anchor);
   d.url.push_back(String("github"));
   d.url.push_back(String("398"));
   queueLock.lock();
   docList.push_back(d);
   queueLock.unlock();
   while(true){}
}
