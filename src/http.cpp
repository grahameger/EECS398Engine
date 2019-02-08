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

    std::string HTTPRequest::filename() {
        auto s = host + path + query;
        std::replace(s.begin(), s.end(), '/', '_');
        return s;
    }

    std::string HTTPRequest::requestString() {
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

    void HTTPResponse::writeToFile(HTTPRequest * request) {
        std::ofstream f(request->filename());
        auto s = data.str();
        f.write(s.c_str(), s.size());
        f.close();
    }

    // do all the processing necessary for the HTTP stream.
    void HTTPResponse::process(HTTPRequest * request) {

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

        // open a socket to the host
        int sockfd = getConnToHost(request->host, request->port);

        // add request state to client data structure.
        m.lock();
        clientInfo[sockfd] = {request, nullptr};
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
                        if (errno == EWOULDBLOCK) {
                            io.addSocket(sockfd);
                            continue;
                        }
                        else if (errno == ECONNRESET) {
                            info.response->writeToFile(info.request);
                            break;
                        }
                        else {
                            printf("Error with recv(), %s\n", strerror(errno));
                        }

                    }
                    // actually write it to the HTTPResponse struct.
                    if (info.response == nullptr) {
                        info.response = new HTTPResponse;
                    }
                    info.response->data.write(buffer, bytes_read);
                    if (bytes_read == 0) {
                        // socket closed on the other end. Write to file
                        info.response->writeToFile(info.request);
                        break;
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

    void HTTPClient::parseResponse(int sockfd, ClientInfo &info) {
        // it's been written to the HTTPResponse
        if (info.response->header_length == -1 || info.response->content_length == -1) {
            size_t pos = info.response->data.str().find("\r\n\r\n");
            if (pos != std::string::npos) {
                info.response->header_length = pos + 4;
                size_t leftovers = info.response->data.str().size() - info.response->header_length;
                std::string word = "";
                while (word != "Content-Length") {
                    info.response->data >> word;
                }
                info.response->data >> word;
                info.response->content_length = std::atoi(word.c_str()) - leftovers; 
            }
        }
        if (info.response->header_length + info.response->content_length 
                == info.response->data.str().size()) {
            // we're done downloading this file we can process it
            // info.response->process(info.request);
            info.response->writeToFile(info.request);
            close(sockfd);
        } else {
            io.addSocket(sockfd);
        }
    }
}
