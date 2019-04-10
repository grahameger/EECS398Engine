//
//  crawler.hpp
//  engine
//
//  Created by Graham Eger on 2/14/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//
// Base Crawler Class

#pragma once
#ifndef EECS398_CRAWLER_H
#define EECS398_CRAWLER_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <mutex>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <stdio.h>

#include "RobotsTxt.h"
#include "http.h"
#include "threading.h"
#include "BloomFilter.h"
#include "File.h"

class RobotsTxt;

namespace search {
    class HTTPClient;
    void makeDir(const char * name);
    class Crawler {
    public: 
        Crawler(const std::vector<std::string> &seedUrls);
        ~Crawler();
        void * stub();
        void * print();
        static void * stubHelper(void * context);
        static void * printHelper(void * context);
        bool haveRobots(const std::string &domain);
        bool havePage(const HTTPRequest &req);
        bool killedPage(const HTTPRequest &req);
        void addPageToFilter(const HTTPRequest &req);

        static void domainLock();
        static void domainUnlock();

        static const size_t NUM_CRAWLER_THREADS = 500;
        static const size_t DOMAIN_REHIT_WAIT_TIME = 5;
    private:
        friend class HTTPClient;
        threading::ThreadQueue<std::string> readyQueue;
        pthread_t threads[NUM_CRAWLER_THREADS];
        pthread_t printThread;
        HTTPClient * client;
        inline static pthread_mutex_t domainMutex;
        RobotsTxt * robots;
        std::unordered_map<std::string, time_t> lastHitHost;
        
        // when a page is really bad and we don't want to crawl it again, it meets the killFilter
        BloomFilter<std::string> killFilter; 
        static const size_t killFilterSize; // TODO: we need to write the bad pages to disk somewhere too
        std::map<std::string, std::set<std::string> > waitingForRobots;
        pthread_mutex_t waitingForRobotsLock;
    };
}

#endif