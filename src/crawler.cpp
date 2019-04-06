//
//  crawler.cpp
//  engine
//
//  Created by Graham Eger on 2/14/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//
// Base Crawler Class
// Updated by Graham Eger a lot. 

#include "crawler.h"

namespace search {

    Crawler::Crawler(const std::vector<std::string> &seedUrls) {

        // get our HTTP client
        client = new HTTPClient(this);

        // bloom filter, 1GB
        pageFilter = BloomFilter<std::string>(1000000000);
        // we should probably initialize it
        initializePageFilter();

        // initialize mutex
        domainMutex = PTHREAD_MUTEX_INITIALIZER;
        robots = &threading::Singleton<RobotsTxt>::getInstance();

        // make the robots and pages directory
        makeDir("robots");
        makeDir("pages");

        // initialize our directory hierarchy for pages
        // !nah we're not doing this we're going to use ext4
        // specifically so we don't have to do this
        // O(1) file access, creation in a directory 
        // with an unbounded number of files (2^32-1 really)
         
        // add the seed urls to the queue
        readyQueue.push(seedUrls);

        // when does this run?
        for (size_t i = 0; i < NUM_CRAWLER_THREADS; i++) {
            pthread_create(&threads[i], NULL, &Crawler::stubHelper, this);
        }
    }

    // only do it for non robots pages
    void Crawler::initializePageFilter() {
        DIR * d;
        struct dirent * dir;
        d = opendir("pages/");
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                std::string fileName = std::string(d->d_name);
                pageFilter.add(filName);
            }
            closedir(d);
        }
    }

    void makeDir(const char * name) {
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

    // only go to the filesystem if we really have to
    bool Crawler::havePage(const HTTPRequest &req) {
        // parse the page into a request and get a filename
        auto filename = HTTPRequest(url).filename();
        if (pageFilter.exists(filename)) {
            // check the filesystem only if the bloom filter says we shouldn't
            // TODO: performance measurement on filesystem access vs. the cache
            // thrashing of using the bloom filter.
            struct stat st = {0};
            return (stat(path.c_str(), &st) == 0);
        } else {
            return false;
        }
    }

    void Crawler::addPageToFilter(const HTTPRequest &req) {
        auto filename = req.filename();
        pageFilter.add(filename);
    }

    void * Crawler::stub() {
        while (true) {
            std::string p = readyQueue.pop();
            auto req = HTTPRequest(p);

            // check if we have the robots file for this domain
            if (!haveRobots(req.host)) {
                // change the path to get the robots.txt file
                req.path = "/robots.txt";
                // add the old url to the back of the queue until we get the robots file
                auto newUrl = req.uri();
                // TODO: URLs that fail to get a robots.txt file may pile up at the back of the
                // queue, we need to have a method to get rid of those. Perhaps a separate queue.
                readyQueue.push(p);
                client->SubmitURLSync(newUrl, 0);
                continue;
            }

            // check the domain timer, we want to wait
            // WAIT_TIME seconds between pages on the same host
            pthread_mutex_lock(&domainMutex);
            auto it = lastHitHost.find(req.host);
            if (it != lastHitHost.end()) {
                if (difftime(time(NULL), it->second) > DOMAIN_REHIT_WAIT_TIME) {
                    // reset the time
                    it->second = time(NULL);
                    // unlock mutex
                    pthread_mutex_unlock(&domainMutex);
                    // submitURLSync();
                    client->SubmitURLSync(p, 0);
                    // continue
                    continue;
                } else {
                    // unlock mutex
                    pthread_mutex_unlock(&domainMutex);
                    // add the page to the back of the Queue
                    readyQueue.push(p);
                    // continue
                    continue;
                }
            } else {
                // insert page to the hash table
                lastHitHost.insert({req.host, time(NULL)});
                // unlock mutex
                pthread_mutex_unlock(&domainMutex);
                // submitURLSync
                client->SubmitURLSync(p, 0);
                // continue
                continue;
            }
        }
    }

    void * Crawler::stubHelper(void * context) {
        return (((Crawler *)context)->stub());
    }

    // just join and never ever quit
    Crawler::~Crawler() {
        for (size_t i = 0; i < NUM_CRAWLER_THREADS; i++)
        {
            pthread_join(threads[i], NULL);
        }
    }

    inline void Crawler::domainLock() {
        pthread_mutex_lock(&domainMutex);
    }

    inline void Crawler::domainUnlock() {
        pthread_mutex_unlock(&domainMutex);
    }
}
