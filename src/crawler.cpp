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
    Crawler::Crawler(const std::vector<std::string> &urls_in) {
        for (auto &i : urls_in) {
            urls.push(i);
        }
    }

    void Crawler::SubmitOne(const std::string &url) {
        pthread_t t;
        SubmitArgs * args = new SubmitArgs;
        args->client = &client;
        args->url = new std::string(url);
        pthread_create(&t, NULL, &HTTPClient::SubmitUrlSyncWrapper, (void *)args);
    }
}
