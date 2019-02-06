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

    // parse a well formed url and get the stuff within
    // URI = scheme:[//authority]path[?query][#fragment]
    // authority = [userinfo@]host[:port]
    // uses the RFC 3986 regex suggestion for URL parsing
    HTTPRequest * parseURL(const std::string &url) {
        std::regex r(
                R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
                std::regex::extended
        );
        std::smatch result;
        if (std::regex_match(url, result, r)) {
            HTTPRequest * rv = new HTTPRequest;
            rv->protocol = result[2];
            rv->host =     result[4];
            rv->path =     result[5];
            rv->query =    result[7];
            rv->fragment = result[9];
            return rv;
        } else {
            return nullptr;
        }
    }

    // do all the processing necessary for the HTTP stream.
    void HTTPResponse::process(int fd) {
        // for now we're just going to write the whole stream to the file
        data.seekg(0, std::ios::end);
        auto size = data.tellg();
        data.seekg(0, std::ios::beg);
        write(fd, data.str().c_str(), size);
    }
    
    // send an entire buffer
    static bool sendall(int sockfd, const char * buf, size_t len) {
        while (len > 0) {
            auto i = send(sockfd, buf, len, O_NONBLOCK | MSG_NOSIGNAL);
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
        io.addSocket(sockfd);
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
        // set socket to non blocking
        int rv = fcntl(sockfd, F_SETFL, O_NONBLOCK);
        if (rv == -1) {
            // TODO: log error
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
        // worker threads
        for (size_t i = 0; i < NUM_THREADS; i++) {
            threads[i] = std::thread(&HTTPClient::processResponses, this);
        }
    }

    HTTPClient::~HTTPClient() {
        for (size_t i = 0; i < NUM_THREADS; i++) {
            threads[i].join();
        }
    }

    // 'main' function our worker threads run
    // everything in here will be completely cross platform
    // TODO still some funkiness. Working on the organizing thread stuff. 
    void HTTPClient::processResponses() {
        char buffer[BUFFER_SIZE];
        while (true) {
            int sockfd = io.getSocket();
            m.lock();
            auto iter = clientInfo.find(sockfd);
            if (iter != clientInfo.end()) {
                ClientInfo& info = clientInfo.at(sockfd);
                m.unlock();
                // recieve on the socket while there's data
                ssize_t bytes_read = 0;
                do
                {
                    bzero(buffer, sizeof buffer);
                    bytes_read = recv(sockfd, buffer, sizeof buffer, O_NONBLOCK);
                    if (bytes_read == -1) {
                        // socket closed or other issue
                        // TODO: actual error handling, currently SIGPIPE is completely
                        // ignored so we have to still deal with that.
                        break;
                    }
                    if (bytes_read == 0) {
                        // socket closed on the other end. Do some processing
                        if (info.response != nullptr) {
                            info.response->process(info.fd);
                        }
                        removeFd(sockfd);
                    }
                    // actually write it to the HTTPResponse struct.
                    if (info.response == nullptr) {
                        info.response = new HTTPResponse;
                        info.response->data.write(buffer, bytes_read);
                    } 
                } while (bytes_read > 0);
            }
            else {
                m.unlock();
                // TODO: log
                continue;
            }
        }
    }
}