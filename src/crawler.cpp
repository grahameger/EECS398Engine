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
#include <dirent.h>

namespace search {

    Crawler::Crawler(const std::vector<std::string> &seedUrls) {

        // get our HTTP client
        client = new HTTPClient(this);

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

    void makeDir(const char * name) {
        struct stat st = {0};
        if (stat(name, &st) == -1) {
            int rv = mkdir(name, 0755);
            if (rv == -1) {
                fprintf(stderr, "error creating directory %s - %s\n", name, strerror(errno));
                exit(1);
            } else {
                fprintf(stdout, "created directory %s\n", name);
            }
        }
    }

    inline bool Crawler::haveRobots(const std::string &host) {
        struct stat st = {0};
        auto path = std::string("robots/" + host);
        return (stat(path.c_str(), &st) == 0);    
    }

    bool Crawler::havePage(const HTTPRequest& req) {
        std::string filename = req.filename();
        return File::find(filename.c_str()).exists();
    }

    void * Crawler::stub() {
        while (true) {
            std::string p = readyQueue.pop();
            auto req = HTTPRequest(p);

            // no duplicates, we're only going to be checking this here to
            // prevent going to the filesystem twice.
            // Once we we add it to the queue and once when we pop from the queue
            if (havePage(req)) {
                continue;
            }

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
