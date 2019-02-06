//
//  http.hpp
//  engine
//
//  Created by Graham Eger on 2/1/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#ifndef http_hpp_398
#define http_hpp_398

#include <stdio.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <sstream>
#include <fcntl.h>
#include <mutex>
#include <thread>
#include <sys/types.h>
#include <vector>
#include <array>
#include <unordered_set>
#include <sys/socket.h> 
#include <signal.h>
#include <netinet/in.h> 
#include <netdb.h> 
#include <sys/stat.h>
#include <unistd.h> 

#include "event.hpp"


#ifdef __linux__ 
#include <sys/epoll.h>
#else
#include <sys/types.h>
// http://www.manpagez.com/man/2/kevent/
#include <sys/event.h>
#include <sys/time.h>
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

namespace search {
    struct HTTPRequest
    {
        // can optimize this later
        std::string filename();
        std::string requestString();
        void print();
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
        void process(int fd);
        std::stringstream data;
        int code;
        std::string mimeType;
        std::string encoding;
    };

    struct ClientInfo {
        ~ClientInfo() {
            if (response) {
                delete response;
            }
        }
        int fd; 
        int sockfd;
        HTTPResponse * response;
    };

    class HTTPClient {
    public:
        HTTPClient();
        ~HTTPClient();
        void SubmitURL(const std::string &url);
    private:
        static const size_t MAX_CONNECTIONS = 1000;
        static const size_t RECV_SIZE = 2048;
        static const size_t BUFFER_SIZE = RECV_SIZE;
        static const size_t NUM_THREADS = 4;
        static const uint32_t SLEEP_US = 10000;

        // returns connected TCP socket to host
        int getConnToHost(const std::string &host, int port);

        // 'main' function our worker threads run
        void processResponses();

        // given a socket return the clientInfo
        std::unordered_map<int, ClientInfo> clientInfo;
        std::mutex m;
        std::thread threads[NUM_THREADS];
        search::EventQueue io;

    };
}

#endif /* html_hpp */
