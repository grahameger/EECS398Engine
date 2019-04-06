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
#include <unordered_map>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <mutex>

#include "RobotsTxt.h"
#include "http.h"
#include "threading.h"

class RobotsTxt;

namespace search {
    class HTTPClient;

    void makeDir(const char * name);
    class Crawler {
    public: 
        Crawler(const std::vector<std::string> &seedUrls);
        ~Crawler();

        void * stub();
        static void * stubHelper(void * context);


        bool haveRobots(const std::string &domain);

        static void domainLock();
        static void domainUnlock();

        static const size_t NUM_CRAWLER_THREADS = 1;
        static const size_t DOMAIN_REHIT_WAIT_TIME = 3;
    private:
        friend class HTTPClient;
        threading::ThreadQueue<std::string> readyQueue;
        pthread_t threads[NUM_CRAWLER_THREADS];

        HTTPClient * client;

        inline static pthread_mutex_t domainMutex;

        RobotsTxt * robots;
        
        std::unordered_map<std::string, time_t> lastHitHost;
    };
}

#endif