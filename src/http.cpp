//
//  http.cpp
//  engine
//
//  Created by Graham Eger on 1/26/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#include "http.h"
#include <iostream>
#include <string>
#include <sstream>
#include <regex>
#include <memory>
#include <charconv>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

namespace search {

    // returns true if the given url has already been fetched
    bool alreadyFetched(const std::string &url) {
        auto filename = HTTPRequest(url).filename();
        struct stat buffer;
        return (stat (filename.c_str(), &buffer) == 0);
    }

    // returns a connected socket, -1 if error
    int HTTPClient::getConnToHost(const std::string &host, int port, bool blocking) {
        std::string portStr = std::to_string(port);
        struct addrinfo hints, *servinfo, *p;
        servinfo = p = nullptr;
        int sockfd = 0;
        int rv = 0;
        // load up address structs with getaddrinfo();
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        if ((rv = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo returned %d for host '%s' -- %s -- %s\n", rv, host.c_str(), gai_strerror(rv), strerror(errno));
            if (rv == -11) {
                rv = 0;
            }
            return -1;
        }
        for (p = servinfo; p != nullptr; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                // TODO log error
                continue;
            }
            if (!blocking) {
                rv = fcntl(sockfd, F_SETFL, O_NONBLOCK);
                if (rv == -1) {
                    fprintf(stderr, "could not set socket to non-blocking for host '%s', strerror: %s\n", host.c_str(), gai_strerror(rv));
                    close(sockfd);
                    return -1;
                }
            }
            // timeout section of the show
            timeval tm = {constants::TIMEOUTSECONDS, constants::TIMEOUTUSECONDS};
            if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (timeval*)&tm, sizeof(tm)) == -1) {
                fprintf(stderr, "setsockopt failed for host '%s', strerror: %s\n", host.c_str(), strerror(errno));
                close(sockfd);
                return -1;
            }

            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                // TODO log connect error
                close(sockfd);
                continue;
            }
            break;
        }
        // end of list with no connection
        if (p == nullptr) {  
            // TODO: log, failed to connect
            fprintf(stderr, "unable to connect to host '%s'\n", host.c_str());
            return -1;
        }
        freeaddrinfo(servinfo);
        return sockfd;
    }

    HTTPClient::HTTPClient() {
        robots = &threading::Singleton<RobotsTxt>::getInstance();

        // this will become a bug if there is ever more than
        // one instance of HTTP client.
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        static const SSL_METHOD * meth = TLS_client_method();

        search::HTTPClient::sslContext = SSL_CTX_new(meth);

        // cross platform stuff
        signal(SIGPIPE, SIG_IGN);
    }

    HTTPClient::~HTTPClient() {
        // SSL Stuff, shouldn't run until all the threads return!
        destroySSL();
    }

    void HTTPClient::destroySSL() {
        ERR_free_strings();
        EVP_cleanup();
    }

    static ssize_t getContentLength(std::string_view response) {
        static const std::string CONTENT_LENGTH_STR = "Content-Length: ";
        size_t contentLenStart = response.find(CONTENT_LENGTH_STR);
        if (contentLenStart == std::basic_string_view<char>::npos)
            return -1;
        size_t intStart = std::string_view::npos;
        size_t intEnd = intStart;

        size_t i;
        // get the start of the integer inclusize
        for (i = contentLenStart; i < response.size(); i++) {
            if (std::isdigit(response[i])) {
                intStart = i;
                ++i;
                break;
            }
        }
        // get the end of the integer
        for (; i < response.size(); i++) {
            if (!std::isdigit(response[i])) {
                intEnd = i;
                break;
            }
        }
        size_t rv = -1;
        std::from_chars(response.data() + intStart,
                                      response.data() + intEnd,
                                      rv);
        return rv;
    }

    void HTTPClient::SubmitURLSync(const std::string &url, size_t redirCount) {
        if (redirCount > REDIRECT_MAX) {
            fprintf(stderr, "Too many redirects ending in host %s", url.c_str());
            return;
        }
        HTTPRequest request(url);
        std::unique_ptr<Socket> sock;
        request.method = constants::getMethod;
        request.headers = constants::connClose;
        if (request.protocol == constants::httpsStr) {
            request.port = 443;
            sock = std::unique_ptr<SecureSocket>(new SecureSocket);
        }
        else if (request.protocol == constants::httpStr) {
            request.port = 80;
            sock = std::unique_ptr<Socket>(new Socket);
        }
        else {
            // invalid, TODO log bad request
            return;
        }
        // open a socket to the host
        int sockfd = getConnToHost(request.host, request.port , true);
        if (sockfd < 0) {
            return; 
        }
        ssize_t rv = sock->setFd(sockfd);
        if (rv < 0) {
            fprintf(stderr, "error setting file descriptor for host '%s' : %s", url.c_str(), strerror(errno));
            return;
        }

        rv = 0;
        int bytesReceived = 0;
        size_t totalSize = 0;
        ssize_t contentLength = -1;
        char * fullResponse = (char*)malloc(constants::BUFFER_SIZE);

        // send request blocking
        const std::string requestStr = request.requestString();
        if (sock->send(requestStr.c_str(), requestStr.size()) == -1) {
            fprintf(stderr, "error sending request to host for url '%s'", url.c_str());
        }

        // dynamic buffering
        // every time recv returns we'll look for "Content-Length", length of the body
        // when we get t he length of the body then we can have a hard coded size to check for
        // get the size of the header by searching for /r/n/r/n
        while (true) {
            rv = sock->recv(fullResponse + bytesReceived, constants::BUFFER_SIZE - bytesReceived, 0);
            if (rv < 0) {
                // error check
                fprintf(stderr, "recv returned an error for url '%s'\n", url.c_str());
                free(fullResponse);
                return;
            } else if (rv == 0) {
                // handle EOF
                break;
                // might need to do more?
            } else if (contentLength == -1) {
                // check if the header has been downloaded
                bytesReceived += rv;
                std::string_view view(fullResponse, bytesReceived);
                size_t header_pos = view.find("\r\n\r\n");
                if (header_pos != std::string_view::npos) {
                    size_t header_size = header_pos + 4;
                    // search for "Content-Length"
                    contentLength = getContentLength(std::string_view(fullResponse, bytesReceived));
                    totalSize = header_size + contentLength;
                    ssize_t remaining = totalSize - bytesReceived;
                    if (remaining > 0) {
                        fullResponse = (char *)realloc(fullResponse, totalSize);
                        char * buf_front = fullResponse + bytesReceived;
                        rv = sock->recv(buf_front, remaining, MSG_WAITALL);
                        if (rv >= 0) {
                            bytesReceived += rv;
                            break;
                        }
                        if (rv < 0 || bytesReceived < contentLength) {
                            // error reading from socket
                            break;
                        }
                    }
                }
            } else {
                // no content length found?
                break;
            }
        }

        char * redirectUrl = checkRedirectsHelper(fullResponse, bytesReceived);
        if (redirectUrl) {
            free(fullResponse);
            return SubmitURLSync(redirectUrl, ++redirCount);
        }

        process(fullResponse, bytesReceived);

        // either going to write to a file or add another request to the queue
        // write it to a file
        std::string filename = request.filename();
        std::ofstream outfile(filename);
        outfile.write(fullResponse, totalSize);
        outfile.close();
        free(fullResponse);
        fprintf(stdout, "wrote: %s to disk.\n", filename.c_str());

        if (request.robots()) {
            // TODO: make the data structure itself thread safe in a
            // more optimized way than doing this...
            robots->lock();
            robots->SubmitRobotsTxt(request.host, filename);
            robots->unlock();
        }
    }

    void HTTPClient::process(char * file, size_t len) {

    }

    char * HTTPClient::checkRedirectsHelper(const char * getMessage, const size_t len) {
        if (len < 10) {
            return nullptr;
        }
        const char& redirectLeadInt = getMessage[9];
        if (redirectLeadInt == '3'){
            //Convert GET message to string for ease of access
            std::string messageCopy(getMessage);
            std::string currentLine;
            std::string redirURL;
            //Parse GET message
            for (size_t i = 0; i < messageCopy.length(); ++i){
                //Found the redirected link URL
                if (currentLine == "Location: " || currentLine == "location: "){
                    while (messageCopy[i] != '\n'){
                        redirURL += messageCopy[i];
                        i++;
                    }
                    break;
                }
                //Reset parsed line if newline char found
                else if (messageCopy[i] == '\n'){
                    currentLine = "";
                }
                //Append GET message to curr_line to scan for 'Location: ' string
                else {
                    currentLine += messageCopy[i];
                }
            }
            redirURL.erase(std::remove_if(redirURL.begin(), redirURL.end(), [](char ch) {
                return std::isspace(ch);
            }), redirURL.end());
            char * finalUrl = (char*)calloc(redirURL.length() + 1, sizeof(char));
            strcpy(finalUrl, redirURL.c_str());
            return finalUrl;
        }
        else {
            return nullptr;
        }
    }

    int HTTPClient::Socket::setFd(int fd_in) {
        if (fd_in < 3) {
            return -1;
        }
        sockfd = fd_in;
        return sockfd;
    }

    int HTTPClient::SecureSocket::setFd(int fd_in) {
        if (fd_in < 3) {
            return -1;
        }
        sockfd = fd_in;
        ssl = ::SSL_new(search::HTTPClient::sslContext);
        if (!ssl) {
            // log error
        }
        ::SSL_set_fd(ssl, sockfd);
        int rv = ::SSL_connect(ssl);
        if (rv <= 0) {
            // TODO better error handling
            fprintf(stderr, "Error creating SSL connection");
            fflush(stderr);
        }
        return rv;
    }

    ssize_t HTTPClient::Socket::send(const char * buf, size_t len) {
        while (len > 0) {
            auto i = ::send(sockfd, (void*)buf, len, MSG_NOSIGNAL);
            if (i < 1) {
                return i;
            }
            buf += i;
            len -= i;
        }
        return 0;
    }
    ssize_t HTTPClient::SecureSocket::send(const char * buf, size_t len) {
        auto rv = SSL_write(ssl, buf, len);
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
                    return 0;
            }
        } else {
            return len;
        }
        return rv;
    }

    ssize_t HTTPClient::Socket::recv(char * buf, size_t len, int flags) {
        ssize_t rv = ::recv(sockfd, (void*) buf, len, flags);
        return rv;
    }
    
    ssize_t HTTPClient::SecureSocket::recv(char * buf, size_t len, int flags) {
        if (flags & MSG_WAITALL) {
            size_t bytes_received = 0;
            size_t rv = 0;
            while ((rv = ::SSL_read(ssl, (void*)(buf + bytes_received), len - bytes_received)) > 0) {
                bytes_received += rv;
            }
            if (rv < 0) {
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
                        return 0;
                }
                return -1;
            }
            return bytes_received;
        // This is probably an unnecessary branch. We're doing blocking on all sockets
        // currently
        } else {
            auto rv = ::SSL_read(ssl, (void*)buf, len);
            // todo error handling
            // https://linux.die.net/man/3/ssl_read
            return rv;
        }
    }

    ssize_t HTTPClient::Socket::close() {
        if (sockfd > 2) {
            int rv = ::close(sockfd);
            sockfd = -1;
            return rv;
        }
        return 0;
    }
    ssize_t HTTPClient::SecureSocket::close() {
        // TODO error handling
        if (ssl) {
            ::SSL_shutdown(ssl);
            ::SSL_free(ssl);
            ssl = nullptr;
        }
        if (sockfd > 2) {
            int rv = ::close(sockfd);
            sockfd = -1;
            return rv;
        }
        return 0;
    }
    
}
