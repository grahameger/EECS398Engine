//
//  crawler.cpp
//  engine
//
//  Created by Graham Eger on 2/14/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//
// Base Crawler Class

#include "crawler.hpp"

<<<<<<< HEAD

//to check before crawling add to here
=======
>>>>>>> 57e88919d64a931761332d7fb7828a3faf66dbad
namespace search {
    Crawler::Crawler(const std::string &directory, const std::vector<std::string> &urls_in) {
        watchDir = directory;

        // inotify stuff
        inotifyfd = inotify_init();
        wd = inotify_add_watch(inotifyfd, directory.c_str(), IN_CREATE);

        for (auto &i : urls_in) {
            urls.push(i);
        }
        pthread_create(&watchThread, nullptr, &Crawler::watchForFilesWrapper, (void*) this);
    }

    void * Crawler::watchForFilesWrapper(void * context) {
        ((Crawler *)context)->watchForFiles();
        return nullptr; 
    }

    void Crawler::SubmitOne(const std::string &url) {
        pthread_t t;
        SubmitArgs * args = new SubmitArgs;
        args->client = &client;
        args->url = new std::string(url);
        pthread_create(&t, NULL, &HTTPClient::SubmitUrlSyncWrapper, (void *)args);
    }

    void Crawler::watchForFiles() {
        // use our Eventing class which is no longer in use
        while (true) {
            char buffer[INOTIFY_BUFFER_SIZE];
            int length = read(inotifyfd, buffer, INOTIFY_BUFFER_SIZE);
            if (length < 0) {
                perror("read");
                // TODO, better.
            }
            int i = 0;
            while (i < length) {
                inotify_event * event = (inotify_event * ) &buffer[i];
                if (event->len && (event->mask & IN_CREATE) && !(event->mask & IN_ISDIR)) {
                    // file created with name and filename length
                    event->name;
                    event->len;
                }
                i += EVENT_SIZE + event->len;
            }
        }
    }

}
