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
#include <atomic>
#include <optional>

#include "Parser.hpp"
#include "RobotsTxt.h"
#include "http.h"
#include "threading.h"
#include "BloomFilter.h"

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
        void print2(double &prevGiB, time_t& prevTime);
        static void * stubHelper(void * context);
        static void * printHelper(void * context);
        bool haveRobots(const std::string &domain);
        bool havePage(const HTTPRequest &req);
        bool killedPage(const HTTPRequest &req);
        void addPageToFilter(const HTTPRequest &req);

        static void domainLock();
        static void domainUnlock();

        static const size_t NUM_CRAWLER_THREADS = 1;
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
        BloomFilter<std::string> pageFilter;
        std::set<std::string> robotsDomains;
        std::map<std::string, std::set<std::string> > waitingForRobots;
        inline static pthread_mutex_t waitingForRobotsLock;
        std::atomic<size_t> numBytes; 
        std::atomic<size_t> numPages; 
        std::atomic<size_t> numRobots; 
        // std::atomic<size_t> is the only way I've found to use the ISO C11 atomic intrinsics (stdatomic.h) in a C++ class 
        // Without actually handwriting the assembly. You can sort of apply extern "C" to a member function via a very convoluted hack,
        // but it is not portable to all compiler versions.
    };

    struct MemoryMappedFile {
        char * ptr;
        size_t size;
    };

    std::optional<MemoryMappedFile> memoryMapFile(const std::string &filename);
    void parseFileOnDisk( std::string filename,
                                std::deque<Doc_object>& d,
                                threading::Mutex &m,
                                threading::ConditionVariable& cv);
    
    void parseFiles(std::deque<std::string> filenames,
                    std::deque<Doc_object>& d,
                    threading::Mutex& m,
                    threading::ConditionVariable& cv);
}

#endif