//
//  http.hpp
//  engine
//
//  Created by Graham Eger on 2/1/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#pragma once
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
#include <cctype>
#include <algorithm>
#include <utility>

#include <charconv>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <signal.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <netdb.h> 
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/opensslconf.h>
#if (SSLEAY_VERSION_NUMBER >= 0x0907000L)
# include <openssl/conf.h>
#endif

#include "threading.h"
#include "httpRequest.h"
#include "constants.h"
#include "RobotsTxt.h"
#include "crawler.h"
#include "Parser.hpp"

class RobotsTxt; 

namespace search {
    class Crawler;

    class HTTPClient {
    public:
        HTTPClient(search::Crawler * crawlerIn);
        ~HTTPClient();
        void SubmitURLSync(const std::string &url, size_t redirCount);
        static void * SubmitUrlSyncWrapper(void * context);
        static const size_t CHUNKED = -10;

    private:
        friend class Crawler; 
        static const size_t REDIRECT_MAX = 20;

        // returns connected TCP socket to host
        int getConnToHost(const std::string &host, int port, bool blocking = false);

        // 'main' function our worker threads run
        void processResponses();
        void process(char* file, size_t len, const std::string &currentUri);

        static bool goodMimeContentType(char * str, ssize_t len);
        static bool response200or300(char * str, ssize_t len);
        static bool containsGzip(char * p, size_t len);

        static char * checkRedirectsHelper(const char * getMessage, size_t len);

        // Resolves a relative URL into an absolute path relative to the current request.
        // Returns a nullptr on errors and if the request is to the current document
        static std::string resolveRelativeUrl(const char * baseURi, const char * newUri);

        // given a socket return the clientInfo
        std::mutex m;

        // openSSL functions and state
        // init (called by constructor)
        // teardown (called by destructor)
        void initSSLCtx();
        void destroySSL();

        static inline SSL_CTX * sslContext;

        RobotsTxt * robots;
        Crawler * crawler;
        int logFd;

        struct Socket {
        public:
            Socket() : sockfd(-1) {}

            virtual ~Socket() {
                ::close(sockfd);
            }
            virtual int setFd(int fd_in);
            virtual ssize_t send(const char * buf, size_t len);
            virtual ssize_t recv(char * buf, size_t len, int flags);
            virtual ssize_t close();
        protected:
            int sockfd;
        };
        
        struct SecureSocket : public Socket {
        public:
            SecureSocket() : ssl(nullptr) {}
            virtual ~SecureSocket() {
                if (ssl) {
                    close(); 
                }
            }
            virtual int setFd(int fd_in);
            virtual ssize_t send(const char * buf, size_t len);
            virtual ssize_t recv(char * buf, size_t len, int flags);
            virtual ssize_t close();
        private:
            SSL * ssl;
            inline static threading::Mutex m;
        };
    };
}

#endif /* html_hpp */
