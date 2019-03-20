//
//  crawler.hpp
//  engine
//
//  Created by Graham Eger on 2/14/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//
// Base Crawler Class

#ifndef crawler_hpp_398
#define crawler_hpp_398

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

namespace search {

    struct Page {
        std::string url;
    };

    class Crawler {
    public: 
        Crawler(const std::vector<std::string> &seedUrls);
        ~Crawler();

        void stub();
        static void stubHelper();

        size_t msSinceLastRequest(const std::string &domain);
    private:
        static const size_t NUM_THREADS = 10000;
        static const double WAIT_TIME = 2.0;

        ThreadQueue<Page> q;
        pthread_t threads[NUM_THREADS];

        HTTPClient client;

        std::mutex domainMutex;
        std::unordered_map<std::string, time_t> lastHitHost;
    };
}

#endif