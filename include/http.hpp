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
#include <mutex>
#include <thread>
#include <utility>
#include <vector>
#include <array>
#include <fstream>
#include <algorithm>

#include <fcntl.h>
#include <sys/types.h>
#include <unordered_set>
#include <sys/socket.h> 
#include <signal.h>
#include <netinet/in.h> 
#include <netdb.h> 
#include <sys/stat.h>
#include <unistd.h> 
#include <sys/epoll.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/opensslconf.h>
#if (SSLEAY_VERSION_NUMBER >= 0x0907000L)
# include <openssl/conf.h>
#endif

#include "event.hpp"



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
        HTTPResponse() : header_length(-1), content_length(-1) {}
        void process(HTTPRequest * request);
        void writeToFile(HTTPRequest * request);
        std::stringstream data;
        ssize_t header_length;
        ssize_t content_length;
        int code;
        std::string mimeType;
        std::string encoding;
    };

    struct ClientInfo {
        HTTPRequest  * request;
        HTTPResponse * response;
        SSL * ssl;
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
        void parseResponse(int sockfd, ClientInfo &info);

        // given a socket return the clientInfo
        std::unordered_map<int, ClientInfo> clientInfo;
        std::mutex m;
        std::thread threads[NUM_THREADS];
        search::EventQueue io;

        // openSSL functions and state
        // init (called by constructor)
        // teardown (called by destructor)
        void initSSLCtx();
        void destroySSL();
        SSL * openSSLConnection(int sockfd);
        void closeSSLConnection(SSL * ssl);
        SSL_CTX * sslContext;
    };
}

#endif /* html_hpp */
