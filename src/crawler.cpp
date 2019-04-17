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
#include <unistd.h>
#include <ctime>

extern volatile sig_atomic_t keep_running;

namespace search {

    Crawler::Crawler(const std::vector<std::string> &seedUrls) : killFilter(1000000000), pageFilter(1000000000) {

        // initialize these or we get funky numbers 
        numPages = 0;
        numRobots = 0;
        numBytes = 0;

        // initialize mutex
        domainMutex = PTHREAD_MUTEX_INITIALIZER;
        waitingForRobotsLock = PTHREAD_MUTEX_INITIALIZER;
        robots = &threading::Singleton<RobotsTxt>::getInstance();

        // make the robots and pages directory
        makeDir("robots");
        makeDir("pages");

        // initialize our directory hierarchy for pages
        // !nah we're not doing this we're going to use ext4
        // specifically so we don't have to do this
        // O(1) file access, creation in a directory 
        // with an unbounded number of files (2^32-1 really)
        client = new HTTPClient(this);

        // add the seed urls to the queue
        pthread_create(&printThread, NULL, &Crawler::printHelper, this);
        for (size_t i = 0; i < NUM_CRAWLER_THREADS; i++) {
            pthread_create(&threads[i], NULL, &Crawler::stubHelper, this);
        }
        fprintf(stderr, "ALL THREADS CREATED: queue size %zu\n", seedUrls.size());
        readyQueue.push(seedUrls);
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

    bool Crawler::havePage(const HTTPRequest& req) {
        auto filename = req.filename();
        // if (pageFilter.exists(req.filename())) {
            // check the filesystem if the bloom filter tells us to
            struct stat st = {0};
            const char * cStr = filename.c_str();
            return (stat(cStr, &st) == 0);
        // }
        // return false;
    }

    bool Crawler::haveRobots(const std::string &host) {
        return true;
        auto path1 = std::string("robots/" + host);
        auto path2 = std::string("robots/" + ("www." + host)); // unfortunately the web sucks
        struct stat st = {0};
        if (robotsDomains.find(path1) != robotsDomains.end() ||
            robotsDomains.find(path2) != robotsDomains.end()) {
            return true;
        }
        if (pageFilter.exists(path1) && (stat(path1.c_str(), &st) == 0)) {
            return true;
        }
        st = {0};
        if (pageFilter.exists(path2) && (stat(path2.c_str(), &st) == 0)) {
            return true;
        }
        return false;
    }

    bool Crawler::killedPage(const HTTPRequest& req) {
        return killFilter.exists(req.uri());
    }

    void * Crawler::stub() {
        while (keep_running) {
            std::string p = readyQueue.pop();
            auto req = HTTPRequest(p);

            // no duplicates, we're only going to be checking this here to
            // prevent going to the filesystem twice.
            // Once we we add it to the queue and once when we pop from the queue
            // Also check the extension to make sure that it's not on the bad list.
            if (havePage(req) || !req.goodExtension()) {
                fprintf(stderr, "[DUPLICATE] %s\n", req.filename().c_str());
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
                    fprintf(stderr, "[NO ROBOTS] %s\n", req.filename().c_str());
                    continue;
                }
                pthread_mutex_unlock(&waitingForRobotsLock);
                req.path = "/robots.txt";
                req.query = "";
                req.fragment = "";
                auto newUrl = req.uri();
                fprintf(stderr, "[SUBMITTING ROBOTS] %s\n", req.filename().c_str());
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
                    fprintf(stderr, "[SUBMITTING] %s\n", req.filename().c_str());
                    client->SubmitURLSync(p, 0);
                    // continue
                    continue;
                } else {
                    // unlock mutex
                    pthread_mutex_unlock(&domainMutex);
                    // add the page to the back of the Queue
                    fprintf(stderr, "[RATE LIMIT] %s\n", req.filename().c_str());
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
                fprintf(stderr, "[SUBMITTING] %s\n", req.filename().c_str());
                client->SubmitURLSync(p, 0);
                // continue
                continue;
            }
        }
        return nullptr;
    }

    void Crawler::print2(double &prevGiB, time_t& prevTime) {
        time_t now = time(0);
        // size and number of files
        size_t pages = numPages;
        size_t bytes = numBytes;
        size_t robots = numRobots;
        const char * time = ctime(&now);
        double GiB = (double)bytes / 1073741824.0;
        double rate = (GiB - (double)prevGiB) / (difftime(now, prevTime)) * 8 * 1024;
        size_t queueSize = readyQueue.size();
        fprintf(stdout, "Time: %s\nGiB downloaded: %f\nRate: %f MBit/s\nTotal Pages: %zu\nTotal Robots: %zu \nItems on Queue: %zu\n\n", time, GiB, rate, pages, robots, queueSize);
        prevTime = now;
        prevGiB = GiB;
    }

    // runs every whatever seconds and prints out the progress of the crawler so far
    void * Crawler::print() {
        double prevGiB = 0;
        time_t oldTime = time(0);
        while (keep_running) {
            for (size_t i = 0; i < 60; i++) 
            {
                // do this once every 10 seconds
                print2(prevGiB, oldTime);
                sleep(10);
            }
            // do everything in this block once every 10 minutes or so
            // remove domains that have not been hit in the last 10 seconds 
            readyQueue.write();
            pthread_mutex_lock(&domainMutex);
            auto now = time(NULL);
            for (auto it = lastHitHost.begin(); it != lastHitHost.end();) {
                if (difftime(now, it->second) > 5.0) {
                    it = lastHitHost.erase(it);
                } else {
                    it++;
                }
            }
            pthread_mutex_unlock(&domainMutex);
        }
        while (keep_running) {
            
        }
        print2(prevGiB, oldTime);
        // need to do this so that any threads that are waiting on the
        // thread queue can pop then quit
        // should make a vector of NUM_CRAWLER_THREADS empty strings and push
        // them all to the queue.
        // Every worker thread will then pop, see that they're empty strings,
        // continue their loop check and see they should exit the loop, and exit
        // then finally all of the destructors will run, and .join will return
        // for every thread.
        // It will take some time because of the blocking IO functions and
        // the potential for many redirects taking time.
        std::vector<std::string> s;
        s.resize(NUM_CRAWLER_THREADS);
        readyQueue.push(s);
        fprintf(stdout, "%s", "Stop signal received, shutting down gracefully\n");
        return nullptr;
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
            fprintf(stderr, "Thread %zu of %zu returned\n", i + 1, NUM_CRAWLER_THREADS);
        }
    }

    inline void Crawler::domainLock() {
        pthread_mutex_lock(&domainMutex);
    }

    inline void Crawler::domainUnlock() {
        pthread_mutex_unlock(&domainMutex);
    }
}
