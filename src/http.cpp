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
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 


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

    std::string HTTPRequest::filename() {
        return host + path + query;
    }

    std::string HTTPRequest::requestString() {
        std::stringstream ss;
        ss << method << ' ' << path << httpVersion << endl;
        ss << hostString << ' ' << host << endl;
        ss << connClose << endl;
        return ss.str();
    }

    // parse a well formed url and get the stuff within
    // URI = scheme:[//authority]path[?query][#fragment]
    // authority = [userinfo@]host[:port]
    // uses the RFC 3986 regex suggestion for URL parsing
    HTTPRequest * parseURL(const std::string &url) {
        std::regex r (
                "(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
                std::regex::extended
        );
        std::smatch result;
        if (std::regex_match(url, result, r)) {
            HTTPRequest * rv = new HTTPRequest;
            rv->host =     result[4];
            rv->path =     result[5];
            rv->query =    result[7];
            rv->fragment = result[9];
            return rv;
        } else {
            return nullptr;
        }
    }
    
    // send an entire buffer
    static bool sendall(int sockfd, const char * buf, size_t len) {
        while (len > 0) {
            auto i = send(sockfd, buf, len, O_NONBLOCK);
            if (i < 1) {
                return false; 
            }
            buf += i;
            len -= i;
        }
        return true;
    }

    void HTTPClient::SubmitURL(const std::string &url) {
        HTTPRequest * request = parseURL(url);
        request->method = getMethod;
        request->headers = connClose;
        if (request->protocol == httpsStr) {
            request->port = 443;
        }
        else if (request->protocol == httpStr) {
            request->port = 80;
        }
        else {
            // invalid, TODO log bad request
            return;
        }
        // create a file that only we can write to
        int fd = open(
                request->filename().c_str(),
                O_RDWR | O_CREAT,
                S_IRUSR | S_IRGRP | S_IROTH
        );

        // open a socket to the host
        int sockfd = getConnToHost(request->host, request->port);

        // add request state to client data structure.
        m.lock();
        clientInfo.insert({
            sockfd, 
            {
                fd,
                sockfd,
                nullptr
            }
        });
        m.unlock();

        // send request non-blocking
        std::string requestStr = request->requestString();
        if (!sendall(sockfd, requestStr.c_str(), requestStr.size())) {
            // TODO: log
        }
    }

    int HTTPClient::getConnToHost(const std::string &host, int port) {
        struct hostent *server;
        struct sockaddr_in serv_addr;
        int sockfd;

        // create socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            // TODO: log
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
        #ifdef __linux__ 
        epollfd = epoll_create1(0);
        #endif
    }

    // adds file descriptor to "watchlist"
    // TODO: currently linux only
    void HTTPClient::addFd(int fd) {
        #ifdef __linux__ 
        struct epoll_event event;
        event.data.fd = fd;
        event.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) < 0) {
            // TODO: log
        }
        #endif
    }

    // gets a ready file descriptor from "watchlist"
    // TODO: currently linux only
    int HTTPClient::getFd() {
        #ifdef __linux__ 
        struct epoll_event event;
        // TODO: change the timeout
        int rv = epoll_wait(epollfd, &event, 1, 1);
        if (rv == -1) {
            // TODO log error
        }
        // TODO make there be more than one event
        return event.data.fd;
        #endif
        return 0;
    }

    // removes a file descriptor from the "watchlist"
    // TODO: currently linux only
    void HTTPClient::removeFd(int fd) {
        #ifdef __linux__
        struct epoll_event event;
        event.data.fd = fd;
        event.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &event) < 0) {
            // TODO: log and error handle
        }
        #endif
    }
}