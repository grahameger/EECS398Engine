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
#include <sys/mman.h>

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
        std::string filename() const;
        std::string requestString() const;
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
    static const HTTPRequest emptyHTTPRequest = HTTPRequest();

    class HTTPClient {
    public:
        HTTPClient();
        ~HTTPClient();
        void SubmitURLSync(const std::string &url);
    private:
        static const size_t MAX_CONNECTIONS = 1000;
        static const size_t RECV_SIZE = 2048;
        static const size_t BUFFER_SIZE = RECV_SIZE;
        static const size_t NUM_THREADS = 4;
        static const uint32_t SLEEP_US = 10000;
        const size_t DEFAULT_FILE_SIZE = 1024000; // 1MiB or 256 pages

        // returns connected TCP socket to host
        int getConnToHost(const std::string &host, int port, bool blocking = false);

        // 'main' function our worker threads run
        void processResponses();
        void process(char* file, size_t len);

        // given a socket return the clientInfo
        std::mutex m;

        // openSSL functions and state
        // init (called by constructor)
        // teardown (called by destructor)
        void initSSLCtx();
        void destroySSL();
        SSL_CTX * sslContext;
    };
}

#endif /* html_hpp */
