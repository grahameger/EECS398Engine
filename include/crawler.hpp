//
//  crawler.hpp
//  engine
//
//  Created by Graham Eger on 2/14/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//
// Base Crawler Class

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <mutex>

#include "http.hpp"
#include "thread_queue.hpp"
#include "semaphore.hpp"
#include "RobotsTxt.h"

namespace search {

    class Crawler {
    public: 
        Crawler(const std::vector<std::string> &seedUrls);
        ~Crawler();

        void * stub();
        static void * stubHelper(void * context);


        bool haveRobots(const std::string &domain);

        static RobotsTxt robots;

        static void robotLock();
        static void robotUnlock();
        static void domainLock();
        static void domainUnlock();

    private:
        static const size_t NUM_THREADS = 10000;
        static const size_t WAIT_TIME = 3;

        threading::ThreadQueue<std::string> q;
        pthread_t threads[NUM_THREADS];

        HTTPClient client;

        static pthread_mutex_t domainMutex;
        static pthread_mutex_t robotsMutex;
        std::unordered_map<std::string, time_t> lastHitHost;
    };
}