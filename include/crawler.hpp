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

#include <sys/inotify.h>
#include <sys/types.h>

#include "http.hpp"
#include "thread_queue.hpp"
#include "semaphore.hpp"

namespace search {
    class Crawler {
    public:
        Crawler(const std::string &directory, const std::vector<std::string> &urls);
        ~Crawler();
        void run();
    private:
        // HTTP stuff
        const size_t MAX_CRAWLER_THREADS = 10000;
        threading::Semaphore sem(10000);
        HTTPClient client;
        threading::ThreadQueue<std::string> urls;
        void SubmitOne(const std::string &url);

        // Crawler Stuff
        threading::ThreadQueue<std::string> filesToProcess;

        // inotify and file watching stuff
        static const size_t EVENT_SIZE = (sizeof (struct inotify_event));
        static const size_t INOTIFY_BUFFER_COUNT = 1024; 
        static const size_t INOTIFY_BUFFER_SIZE = INOTIFY_BUFFER_COUNT * sizeof(inotify_event) + 16; 
        static void * watchForFilesWrapper(void * context);
        void watchForFiles();
        std::string watchDir;
        int inotifyfd;
        int wd; // watch directory fd
        pthread_t watchThread;
    };
}

#endif