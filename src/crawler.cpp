//
//  crawler.cpp
//  engine
//
//  Created by Graham Eger on 2/14/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//
// Base Crawler Class

#include "crawler.hpp"

namespace search {
    Crawler::Crawler() {
        sem = threading::Semaphore(MAX_CRAWLER_THREADS);
    }
}
