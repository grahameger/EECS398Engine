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

    void HTTPClient::SubmitURLSync(const std::string &url) {
        HTTPRequest request = parseURLStack(url);
        Socket * sock;
        request.method = getMethod;
        request.headers = connClose;
        if (request.protocol == httpsStr) {
            request.port = 443;
            sock = new SecureSocket;
        }
        else if (request.protocol == httpStr) {
            request.port = 80;
            sock = new Socket; 
        }
        else {
            // invalid, TODO log bad request
            return;
        }
        // open a socket to the host
        int sockfd = getConnToHost(request.host, request.port);
        sock->setFd(sockfd);
        // send request non-blocking
        const std::string requestStr = request.requestString();
        sock->send(requestStr.c_str(), requestStr.size());

        // dynamic buffering
        // every time recv returns we'll look for "Content-Length", length of the body
        // when we get the length of the body then we can have a hard coded size to check for
        // get the size of the header by searching for /r/n/r/n
        ssize_t rv = 0;
        ssize_t content_length = -1;
        ssize_t current_buf_size = BUFFER_SIZE;
        int bytes_received = 0;
        char * full_response = (char*)malloc(BUFFER_SIZE);

        while (true) {
            sock->recv(full_response, BUFFER_SIZE, 0);
            if (rv < 0) {
                // error check
            } else if (rv == 0) {
                // handle EOF
            } else {
                // check content_length to see if we need to continue or can break
            }
            // check if the header has been downloaded
            if (content_length == -1) {
                char * header_pos = std::find(full_response, full_response + bytes_received, "\r\n\r\n");
                if (header_pos) {
                    size_t header_size = header_pos - full_response + 4;
                    size_t leftover = bytes_received - header_size;
                    // search for Content-Length
                    static const std::string cLen = "Content-Length: ";
                    const char * content_length_pos = std::find(full_response, full_response + header_size, cLen);
                    if (content_length_pos) {
                        // copy data to a new buffer the known size of our full response
                        content_length = std::atoi(content_length_pos + cLen.size());
                        size_t total_size = header_size + 4 + content_length;
                        size_t remaining = total_size - bytes_received;
                        char * tmp = (char*)malloc(total_size);
                        memcpy(tmp, full_response, bytes_received);
                        free(full_response);
                        full_response = tmp;
                        char * buf_front = full_response + bytes_received;
                        rv = sock->recv(full_response, remaining, MSG_WAITALL);
                        if (rv < 0 || bytes_received < content_length) {
                            // error reading from socket
                        } else {
                            // done, break
                            break;
                        }
                    } else {
                        break;
                    }
                }
            }
        }
        // full response is completely downloaded now we can process
        process(full_response, bytes_received);

        // either going to write to a file or add another request to the queue
    }

    void process(char * file, size_t len) {
        // where
    }

    int HTTPClient::Socket::setFd(int fd_in) {
        sockfd = fd_in;
        return sockfd;
    }
    int HTTPClient::SecureSocket::setFd(int fd_in) {
        sockfd = fd_in;
        ssl = ::SSL_new(sslContext);
        if (!ssl) {
            // log error
        }
        int sslsock = ::SSL_get_fd(ssl);
        int rv = ::SSL_connect(ssl);
        if (rv <= 0) {
            // TODO better error handling
            fprintf(stderr, "Error creating SSL connection");
            fflush(stderr);
            return rv;
        }
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
    }

    ssize_t HTTPClient::Socket::recv(char * buf, size_t len, int flags) {
        return ::recv(sockfd, (void*) buf, len, flags);
    }
    
    ssize_t HTTPClient::SecureSocket::recv(char * buf, size_t len, int flags) {
        auto rv = ::SSL_read(ssl, (void*)buf, len);
        // todo error handling
        // https://linux.die.net/man/3/ssl_read
        return rv;
    }

    ssize_t HTTPClient::Socket::close() {
        return ::close(sockfd);
    }
    ssize_t HTTPClient::SecureSocket::close() {
        ::SSL_shutdown(ssl);
        ::SSL_free(ssl);
        ::close(sockfd);
    }
}
