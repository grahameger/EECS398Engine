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

    static void makeDir(const char * name) {
        struct stat st = {0};
        if (stat(name, &st) == -1) {
            mkdir(name, 0700);
        }
    }

    void * Crawler::stub() {
        while (true) {
            auto p = q.pop();
            auto host = client.getHost(p);

            // check the domain timer, we want to wait
            // WAIT_TIME seconds between pages on the same host
            std::unique_lock<std::mutex> lock(domainMutex);
            auto it = lastHitHost.find(host);
            if (it != lastHitHost.end()) {
                if (difftime(time(NULL), &it->second) > WAIT_TIME) {
                    // reset the time
                    it->second = time(NULL);
                    // unlock mutex
                    lock.unlock();
                    // submitURLSync();
                    client.SubmitURLSync(p);
                    // continue
                    continue;
                } else {
                    // unlock mutex
                    lock.unlock();
                    // add the page to the back of the Queue
                    q.push(p);
                    // continue
                    continue;
                }
            } else {
                // insert page to the hash table
                lastHitHost.insert({host, time(NULL)});
                // unlock mutex
                lock.unlock();
                // submitURLSync
                client.SubmitURLSync(p);
                // continue
                continue;
            }
        }
    }

    static void * stubHelper(void * context) {
        return ((Crawler *)context->stub());
    }

    Crawler::Crawler(const std::vector<std::string> &seedUrls) {
        // make the robots and pages directory
        makeDir("robots");
        makeDir("pages");
         
        // add the seed urls to the queue
        q.push(seedUrls);

        // when does this run?
        for (size_t i = 0; i < NUM_THREADS; i++) {
            pthread_create(&threads[i], NULL, &Crawler::stubHelper, this);
        }
    }

    // just join and never ever quit
    Crawler::~Crawler() {
        for (size_t i = 0; i < NUM_THREADS; i++)
        {
            pthread_join(threads[i], NULL);
        }
    }
}
