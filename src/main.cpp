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

/*
static struct option longopts[] = {
   {"indexFilePrefix", required_argument, nullptr, 'i'},
   {"seedList", required_argument, nullptr, 's'},
   {"domainList", required_argument, nullptr, 'd'},
   {0, 0, 0, 0} //default
};
*/
static const char startFile[] = "/home/eger/wiki/urls.txt";
// static const char startFile[] = "/data/crawl/dmoz/dmoz.base.urls";

// static const std::vector<std::string> startFiles = {
//    "/data/crawl/dmoz/dmoz.base.urls",
//    "/data/crawl/reddit/reddit.dedupe.urls",
//    "/data/crawl/hn/HNDump/deduped.hn.urls"
// };

static const size_t NUM_PARSING_THREADS = 24;

FILE * fileOut;

static const unsigned MAXFILES = 500;

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
   
   // open every file in the pages directory
   std::deque<std::string> files;
   std::deque<Doc_object> documents;
   threading::Mutex documentsMutex;
   threading::ConditionVariable documentsFullCv;
   threading::ConditionVariable documentsEmptyCv;
   DIR * dir;
   struct dirent * ent; 
   if ((dir = opendir("pages")) != NULL) {
      // TODO: Remove
      unsigned FilesAdded = 0;
      while (FilesAdded++ != MAXFILES && (ent = readdir(dir)) != NULL) {
         files.push_back(ent->d_name);
      }
      fprintf(stdout, "%u files added\n", FilesAdded);
   }

   fprintf(stdout, "starting to parse\n");

   std::thread threads[NUM_PARSING_THREADS];
   for (size_t i = 0; i < NUM_PARSING_THREADS; ++i) {
      threads[i] = std::thread(search::parseFiles, std::ref(files), std::ref(documents), std::ref(documentsMutex), std::ref(documentsFullCv), std::ref(documentsEmptyCv));
   }
   for (size_t i = 0; i < NUM_PARSING_THREADS; ++i) {
      threads[i].join();
   }
   fprintf(stdout, "done parsing\n");

   Index index(&documents, &documentsMutex, &documentsFullCv, &documentsEmptyCv);

   // Do Nothing
}
