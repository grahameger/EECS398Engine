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
#include <ctime>

namespace search {

    Crawler::Crawler(const std::vector<std::string> &seedUrls) : killFilter(1000000000) {

        // get our HTTP client
        client = new HTTPClient(this);

        // initialize mutex
        domainMutex = PTHREAD_MUTEX_INITIALIZER;
        waitingForRobotsLock = PTHREAD_MUTEX_INITIALIZER;
        robots = &threading::Singleton<RobotsTxt>::getInstance();

        // make the robots and pages directory
        makeDir("robots");

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
        pthread_create(&printThread, NULL, &Crawler::printHelper, this);
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
        auto path1 = std::string("robots/" + host);
        auto path2 = std::string("robots/" + ("www." + host));
        return (stat(path1.c_str(), &st) == 0 || stat(path2.c_str(), &st) == 0);    
    }

    bool Crawler::havePage(const HTTPRequest& req) {
        std::string filename = req.filename();
        const char * cStr = filename.c_str();
        bool rv = File::find(cStr).exists();
        return rv;
    }

    bool Crawler::killedPage(const HTTPRequest& req) {
        return killFilter.exists(req.uri());
    }

    void * Crawler::stub() {
        while (true) {
            std::string p = readyQueue.popRandom();
            auto req = HTTPRequest(p);

            // no duplicates, we're only going to be checking this here to
            // prevent going to the filesystem twice.
            // Once we we add it to the queue and once when we pop from the queue
            // Also check the extension to make sure that it's not on the bad list.
            if (havePage(req) || !req.goodExtension()) {
                continue;
            }

            // check if we have the robots file for this domain
            if (!req.robots() && !haveRobots(req.host)) {
                // change the path to get the robots.txt file
                // add the old url to the back of the queue until we get the robots file
                readyQueue.push(p);
                pthread_mutex_lock(&waitingForRobotsLock);
                // see if there's a set already for it
                auto it = waitingForRobots.find(req.host);
                if (it == waitingForRobots.end()) {
                    // if there isn't create a set and insert it
                    std::set<std::string> s = {req.uri()};
                    waitingForRobots.insert({req.host, s});
                } else {
                    // if there is, just add it to the set
                    it->second.insert(req.uri());
		            pthread_mutex_unlock(&waitingForRobotsLock);
                    continue;
                }
                pthread_mutex_unlock(&waitingForRobotsLock);
                req.path = "/robots.txt";
                auto newUrl = req.uri();
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

    // runs every whatever seconds and prints out the progress of the crawler so far
    void * Crawler::print() {
        double prevGiB = 0;
        while (true) {
            sleep(10);
            // get all the data
            time_t now = time(0);
            // size and number of files
            auto sizeAndNumberOfFiles = File::totalSizeAndNumFiles();
            size_t& number = sizeAndNumberOfFiles.second;
            const char * time = ctime(&now);
            double GiB = (double)sizeAndNumberOfFiles.first / 1073741824.0;
            double rate = (GiB - (double)prevGiB) / 10.0 * 8 * 1024;
            size_t queueSize = readyQueue.size();
            fprintf(stdout, "Time: %s\nGiB downloaded: %f\nRate: %f MBit/s\nTotal Files: %zu\nItems on Queue: %zu\n\n", time, GiB, rate, number, queueSize);
            prevGiB = GiB;
        }
    }

    void * Crawler::stubHelper(void * context) {
        return (((Crawler *)context)->stub());
    }

    void * Crawler::printHelper(void * context) {
        return (((Crawler *)context)->print());
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
