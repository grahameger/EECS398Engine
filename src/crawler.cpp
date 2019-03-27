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

    inline bool Crawler::haveRobots(const std::string &host) {
        struct stat st = {0};
        auto path = std::string("robots/" + host);
        return (stat(path.c_str(), &st) == 0);    
    }

    void * Crawler::stub() {
        while (true) {
            std::string p = q.pop();
            std::string host = getHost(p);

            // check if we have the robots file for this domain
            if (!haveRobots(host)) {
                // get the robots.txt file
                std::string newUrl = "http://" + host + "/robots.txt";
                client.SubmitURLSync(newUrl);
            }

            // check the domain timer, we want to wait
            // WAIT_TIME seconds between pages on the same host
            pthread_mutex_lock(&domainMutex);
            auto it = lastHitHost.find(host);
            if (it != lastHitHost.end()) {
                if (difftime(time(NULL), it->second) > WAIT_TIME) {
                    // reset the time
                    it->second = time(NULL);
                    // unlock mutex
                    pthread_mutex_unlock(&domainMutex);
                    // submitURLSync();
                    client.SubmitURLSync(p);
                    // continue
                    continue;
                } else {
                    // unlock mutex
                    pthread_mutex_unlock(&domainMutex);
                    // add the page to the back of the Queue
                    q.push(p);
                    // continue
                    continue;
                }
            } else {
                // insert page to the hash table
                lastHitHost.insert({host, time(NULL)});
                // unlock mutex
                pthread_mutex_unlock(&domainMutex);
                // submitURLSync
                client.SubmitURLSync(p);
                // continue
                continue;
            }
        }
    }

    void * Crawler::stubHelper(void * context) {
        return (((Crawler *)context)->stub());
    }

    Crawler::Crawler(const std::vector<std::string> &seedUrls) {
        // initialize mutex
        domainMutex = PTHREAD_MUTEX_INITIALIZER;
        robotsMutex = PTHREAD_MUTEX_INITIALIZER;

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

    inline void Crawler::robotLock() {
        pthread_mutex_lock(&robotsMutex);
    }

    inline void Crawler::robotUnlock() {
        pthread_mutex_unlock(&robotsMutex);
    }

    inline void Crawler::domainLock() {
        pthread_mutex_lock(&domainMutex);
    }

    inline void Crawler::domainUnlock() {
        pthread_mutex_unlock(&domainMutex);
    }
}
