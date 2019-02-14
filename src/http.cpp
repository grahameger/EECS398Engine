//
//  http.cpp
//  engine
//
//  Created by Graham Eger on 1/26/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#include "http.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <regex>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <errno.h>
#include <string.h>


namespace search {
    static const std::string getMethod = "GET";
    static const std::string endl = "\r\n";
    static const std::string httpVersion = "HTTP/1.1";
    static const std::string hostString = "Host: ";
    static const std::string connClose = "Connection: close" + endl;
    static const std::string httpStr = "http";
    static const std::string httpsStr = "https";
    static const std::string port80 = "80";
    static const std::string port443 = "443";

    std::string HTTPRequest::filename() const {
        auto s = host + path + query;
        std::replace(s.begin(), s.end(), '/', '_');
        return s;
    }

    std::string HTTPRequest::requestString() const {
        std::stringstream ss;
        ss << method << ' ' << path << ' ' << httpVersion << endl;
        ss << hostString << ' ' << host << endl;
        ss << connClose << endl;
        return ss.str();
    }

    void HTTPRequest::print() {
        std::stringstream ss;
        ss << "{\n";
        ss << "\t" << "method: " << method << '\n';
        ss << "\t" << "host: " << host << '\n';
        ss << "\t" << "path: " << path << '\n';
        ss << "\t" << "query: " << query << '\n';
        ss << "\t" << "fragment: " << fragment << '\n';
        ss << "\t" << "headers: " << headers << '\n';
        ss << "\t" << "protocol: " << protocol << '\n';
        ss << "\t" << "port: " << port << '\n';
        ss << "}\n";
        std::cout << ss.str() << std::flush;
    }
    
    // send an entire buffer
    static bool sendall(int sockfd, const char * buf, size_t len, int flags) {
        while (len > 0) {
            auto i = send(sockfd, buf, len, flags);
            if (i < 1) {
                return false; 
            }
            buf += i;
            len -= i;
        }
        return true;
    }

    int HTTPClient::getConnToHost(const std::string &host, int port, bool blocking) {
        struct hostent *server;
        struct sockaddr_in serv_addr;
        int sockfd;
        // create socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            // TODO: log
        }
        if (!blocking) {
            // set socket to non blocking
            int rv = fcntl(sockfd, F_SETFL, O_NONBLOCK);
            if (rv == -1) {
                // TODO: log error
            }
        }
        // lookup ip address
        server = gethostbyname(host.c_str());
        if (server == nullptr) {
            // TODO: log
        }
        // fill in struct
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(80);
        memcpy(&serv_addr.sin_addr.s_addr,
                server->h_addr,
                server->h_length);
        // connect the socket
        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
            // TODO: log
        }
        return sockfd;
    }

    HTTPClient::HTTPClient() {
        // cross platform stuff
        signal(SIGPIPE, SIG_IGN);

        // SSL stuff
        initSSLCtx();
    }

    HTTPClient::~HTTPClient() {
        // SSL Stuff, shouldn't run until all the threads return!
        destroySSL();
    }

    void HTTPClient::initSSLCtx() {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        static const SSL_METHOD * meth = TLSv1_2_client_method();
        sslContext = SSL_CTX_new(meth);
        // this is deprecated and is potentially unnecessary
        // the OpenSSL wiki says to call it anyway.
        OPENSSL_config(NULL);
        /* Include <openssl/opensslconf.h> to get this define */
        #if defined (OPENSSL_THREADS)
        fprintf(stdout, "Warning: thread locking is not implemented\n");
        #endif
    }

    void HTTPClient::destroySSL() {
        ERR_free_strings();
        EVP_cleanup();
    }

    // parse a well formed url and get the stuff within
    // URI = scheme:[//authority]path[?query][#fragment]
    // authority = [userinfo@]host[:port]
    // uses the RFC 3986 regex suggestion for URL parsing
    // Using GCC 8.2.0 on Linux the return value is moved
    // not copied. Therefore it is faster than the heap
    // based version. 
    // Bad urls will copy the empty request but will not
    // run a bunch of std::string constructors.
    static HTTPRequest parseURLStack(const std::string &url) {
        std::regex r(
                R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
                std::regex::extended
        );
        std::smatch result;
        if (std::regex_match(url, result, r)) {
            HTTPRequest rv;
            rv.protocol = result[2];
            rv.host =     result[4];
            rv.path =     result[5];
            rv.query =    result[7];
            rv.fragment = result[9];
            return rv;
        } else {
            return emptyHTTPRequest;
        }
    }

    SSL * sendSsl(int sockfd, const HTTPRequest &req, SSL_CTX * ctx) {
        SSL * ssl = SSL_new(ctx);
        if (!ssl) {
            // TODO thread safe error logging
            fprintf(stderr, "Error creating SSL.\n");
            return nullptr;
        }
        int sslsock = SSL_get_fd(ssl);
        SSL_set_fd(ssl, sslsock);
        int rv = SSL_connect(ssl);
        if (rv <= 0) {
            fprintf(stderr, "Error creating SSL connection with %s", req.host.c_str());
            fflush(stderr);
            return nullptr;
        }
        auto reqStr = req.requestString();
        rv = SSL_write(ssl, reqStr.c_str(), reqStr.size());
        if (rv <= 0) {
            int error = SSL_get_error(ssl, rv);
            switch (error)
            {
                case SSL_ERROR_WANT_WRITE:
                case SSL_ERROR_WANT_READ:
                    break;
                case SSL_ERROR_ZERO_RETURN:
                case SSL_ERROR_SYSCALL:
                case SSL_ERROR_SSL:
                default:
                    return nullptr;
            }
        }
    }

    void HTTPClient::SubmitURLSync(const std::string &url) {
        HTTPRequest request = parseURLStack(url);
        SSL * ssl = nullptr;
        request.method = getMethod;
        request.headers = connClose;
        if (request.protocol == httpsStr) {
            request.port = 443;
        }
        else if (request.protocol == httpStr) {
            request.port = 80;
        }
        else {
            // invalid, TODO log bad request
            return;
        }
        // open a socket to the host
        int sockfd = getConnToHost(request.host, request.port);
        // send request non-blocking
        const std::string requestStr = request.requestString();
        if (request.port == 443) {
            ssl = sendSsl(sockfd, request, sslContext);
            if (!ssl) {
                // TODO: log
            }
        } else {
            if (!sendall(sockfd, requestStr.c_str(), requestStr.size(), MSG_NOSIGNAL)) {
            // TODO: log
            }
        }
        // open a file
        const int oFlags = O_APPEND | O_CREAT | O_TRUNC | O_RDWR;
        int outputFd = open(request.filename().c_str(), oFlags, S_IWRITE);

        const int mmapFlags = PROT_READ | PROT_WRITE;
        char * map = (char*)mmap(0, DEFAULT_FILE_SIZE, mmapFlags, MAP_SHARED, outputFd, 0);

        size_t currentBufferSize = DEFAULT_FILE_SIZE;
        // receive loop into that memory

        ssize_t rv;
        int bytes_received = 0;
        do {
            // recv blocks and returns if:
            if (ssl) {
                rv = SSL_read(ssl, (void*)(map + bytes_received), currentBufferSize - bytes_received);
            } else {
                rv = recv(sockfd, (void*)(map + bytes_received), currentBufferSize - bytes_received, MSG_WAITALL);
            }             
            if (rv == 0) {
                // EOF
                bytes_received += rv;
            }
            else if(rv < 0) {
                // ERR
                // TODO handle
            }
            else if (rv == DEFAULT_FILE_SIZE) {
                // update bytes received
                bytes_received += rv;
                // unmap
                munmap((void*)map, currentBufferSize);
                // update current buffer size
                currentBufferSize += DEFAULT_FILE_SIZE;
                // reset the file descriptor to the beginning of the file
                off_t currentPos = lseek(outputFd, (size_t)0, SEEK_CUR);
                lseek(outputFd, currentPos, SEEK_SET);
                // mmap
                map = (char*)mmap(0, DEFAULT_FILE_SIZE, mmapFlags, MAP_SHARED, outputFd, 0);
            }
        } while (rv > 0);
        if (ssl)  {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(sockfd);
        }
        process(map, bytes_received);
    }
}
