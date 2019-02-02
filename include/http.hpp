//
//  http.hpp
//  engine
//
//  Created by Graham Eger on 1/26/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#ifndef http_hpp_398
#define http_hpp_398

#include <stdio.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <mutex>
#include <sys/types.h>
#include <vector>
#include <unordered_set>

#ifdef __linux__ 
#include <sys/epoll.h>
#else
#include <sys/event.h>
#endif

namespace search {
    struct HTTPRequest
    {
        // can optimize this later
        std::string filename();
        std::string requestString();
        std::string   method;       // only GET implemented
        std::string   host;
        std::string   path;
        std::string   query;
        std::string   fragment;
        std::string   headers;
        std::string   protocol;
        int           port;         // note 0 defaults to 80
    };

    HTTPRequest * parseURL(const std::string &url);

    struct HTTPResponse 
    {
        char * data;
        int code;
        std::string mimeType;
        std::string encoding;
    };

    struct ClientInfo {
        int fd; 
        int sockfd;
        HTTPResponse * response;
    };

    class HTTPClient {
    public:
        HTTPClient();
        void SubmitURL(const std::string &url);
    private:
        // returns connected TCP socket to host
        int getConnToHost(const std::string &host, int port);

        // adds file descriptor to "watchlist"
        void addFd(int fd);
        // gets a ready file descriptor from "watchlist"
        int getFd();
        // removes a file descriptor from the "watchlist"
        void removeFd(int fd);
        
        #ifdef __linux__ 
        int epollFd;
        #else
        int kq;
        struct kevent chlist[1000];
        struct kevent evlist[1000];
        std::vector<int> sockets;
        std::unordered_set<int>    readySockets; 
        #endif

        // given a socket return the clientInfo
        std::unordered_map<int, ClientInfo> clientInfo;
        std::mutex m;


    };
}

#endif /* html_hpp */
