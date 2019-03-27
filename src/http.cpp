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
#include <memory>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


namespace search {
    static const std::string getMethod = "GET";
    static const std::string endl = "\r\n";
    static const std::string httpVersion = "HTTP/1.1";
    static const std::string hostString = "Host: ";
    static const std::string connClose = "Connection: close" + endl;
    static const std::string userAgent = "User-Agent: Ceatles/1.0 (Linux)";
    static const std::string encoding = "Accept-Encoding: identity";
    static const std::string httpStr = "http";
    static const std::string httpsStr = "https";
    static const std::string port80 = "80";
    static const std::string port443 = "443";

    std::string HTTPRequest::filename() const {
        auto s = host + path + query;
        std::replace(s.begin(), s.end(), '/', '_');
        if (path == "robots.txt") {
            s = "robots/" + s;
        } else {
            s = "pages/" + s;
        }
        return s;
    }

    std::string HTTPRequest::requestString() const {
        std::stringstream ss;
        ss << method << ' ' << path << ' ' << httpVersion << endl;
        ss << hostString << ' ' << host << endl;
        ss << userAgent << endl;
        ss << encoding << endl;
        ss << connClose << endl;
        return ss.str();
    }

    // returns true if the given url has already been fetched
    bool alreadyFetched(const std::string &url) {
        auto filename = parseURLStack(url).filename();
        struct stat buffer;
        return (stat (filename.c_str(), &buffer) == 0);
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
            timeval tm = {TIMEOUTSECONDS, TIMEOUTUSECONDS};
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

    // parse a well formed url and get the stuff within
    // URI = scheme:[//authority]path[?query][#fragment]
    // authority = [userinfo@]host[:port]
    // uses the RFC 3986 regex suggestion for URL parsing
    // Using GCC 8.2.0 on Linux the return value is moved
    // not copied. Therefore it is faster than the heap
    // based version. 
    // Bad urls will copy the empty request but will not
    // run a bunch of std::string constructors.
    HTTPRequest parseURLStack(const std::string &url) {
        static std::regex r(
                R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
                std::regex::extended
        );
        std::smatch result;
        if (std::regex_match(url, result, r)) {
            HTTPRequest rv;
            rv.protocol = result[2];
            rv.host = result[4];
            rv.path = result[5];
            rv.query = result[7];
            rv.fragment = result[9];
            if (rv.path == "")
                rv.path = "/";
            return rv;
        } else {
            return emptyHTTPRequest;
        }
    }

    std::string getHost(const std::string& url) {
        return parseURLStack(url).host;
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

    void HTTPClient::SubmitURLSync(const std::string &url) {
        HTTPRequest request = parseURLStack(url);
        std::unique_ptr<Socket> sock;
        request.method = getMethod;
        request.headers = connClose;
        if (request.protocol == httpsStr) {
            request.port = 443;
            sock = std::unique_ptr<SecureSocket>(new SecureSocket);
        }
        else if (request.protocol == httpStr) {
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
        // send request blocking
        const std::string requestStr = request.requestString();
        if (sock->send(requestStr.c_str(), requestStr.size()) == -1) {
            fprintf(stderr, "error sending request to host for url '%s'", url.c_str());
        }

        // dynamic buffering
        // every time recv returns we'll look for "Content-Length", length of the body
        // when we get t he length of the body then we can have a hard coded size to check for
        // get the size of the header by searching for /r/n/r/n
        rv = 0;
        ssize_t content_length = -1;
        int bytes_received = 0;
        char * full_response = (char*)malloc(BUFFER_SIZE);
        size_t total_size = 0;
        while (true) {
            rv = sock->recv(full_response + bytes_received, BUFFER_SIZE - bytes_received, 0);
            if (rv < 0) {
                // error check
                fprintf(stderr, "recv returned an error for url '%s'\n", url.c_str());
                free(full_response);
                return;
            } else if (rv == 0) {
                // handle EOF
                break;
                // might need to do more?
            } else if (content_length == -1) {
                // check if the header has been downloaded
                bytes_received += rv;
                std::string_view view(full_response, bytes_received);
                size_t header_pos = view.find("\r\n\r\n");
                if (header_pos != std::string_view::npos) {
                    size_t header_size = header_pos + 4;
                    // search for "Content-Length"
                    content_length = getContentLength(std::string_view(full_response, bytes_received));
                    total_size = header_size + content_length;
                    ssize_t remaining = total_size - bytes_received;
                    if (remaining > 0) {
                        full_response = (char *)realloc(full_response, total_size);
                        char * buf_front = full_response + bytes_received;
                        rv = sock->recv(buf_front, remaining, MSG_WAITALL);
                        if (rv >= 0) {
                            bytes_received += rv;
                            break;
                        }
                        if (rv < 0 || bytes_received < content_length) {
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
        // full response is completely downloaded now we can process
        // this should add all the links to the queue
        process(full_response, bytes_received);

        // either going to write to a file or add another request to the queue
        // write it to a file
        auto filename = request.filename();
        std::ofstream outfile(filename);
        outfile.write(full_response, total_size);
        outfile.close();
        free(full_response);
        fprintf(stdout, "wrote: %s to disk.\n", filename.c_str());
    }

    void HTTPClient::process(char * file, size_t len) {
    
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
        //int sslsock = ::SSL_get_fd(ssl);
        // if (sslsock < 0) {
        //     // the operation failed because the underlying BIO
        //     // is not of the correct type
        // }
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
