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

#include "semaphore.hpp"

namespace search {
    class Crawler {
    public:
        Crawler::Crawler();
        Crawler::~Crawler();
    private:
        const static size_t MAX_CRAWLER_THREADS = 10000;
        threading::Semaphore<MAX_CRAWLER_THREADS> sem;
    };
}

#endif