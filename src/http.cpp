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
    static const std::string userAgent = "User-Agent: Ceatles/1.0 (Linux)";
    static const std::string encoding = "Accept-Encoding: identity";
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
        ss << userAgent << endl;
        ss << encoding << endl;
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
        serv_addr.sin_port = htons(port);
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
        // this will become a bug if there is ever more than
        // one instance of HTTP client.
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        static const SSL_METHOD * meth = TLSv1_2_client_method();
        search::HTTPClient::sslContext = SSL_CTX_new(meth);
        // this is deprecated and is potentially unnecessary
        // the OpenSSL wiki says to call it anyway.
        OPENSSL_config(NULL);
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
            if (rv.path == "")
                rv.path = "/";
            return rv;
        } else {
            return emptyHTTPRequest;
        }
    }

    void * HTTPClient::SubmitUrlSyncWrapper(void * context) {
        SubmitArgs * args = (SubmitArgs*)context;
        args->client->SubmitURLSync(*args->url);
        delete args->url;
        delete args;
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
        auto result = std::from_chars(response.data() + intStart,
                                      response.data() + intEnd,
                                      rv);
        return rv;
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
        int sockfd = getConnToHost(request.host, request.port , true);
        sock->setFd(sockfd);
        // send request blocking
        const std::string requestStr = request.requestString();
        sock->send(requestStr.c_str(), requestStr.size());

        // dynamic buffering
        // every time recv returns we'll look for "Content-Length", length of the body
        // when we get the length of the body then we can have a hard coded size to check for
        // get the size of the header by searching for /r/n/r/n
        ssize_t rv = 0;
        ssize_t content_length = -1;
        int bytes_received = 0;
        char * full_response = (char*)malloc(BUFFER_SIZE);
        size_t total_size;
        while (true) {
            rv = sock->recv(full_response, BUFFER_SIZE, 0);
            if (rv < 0) {
                // error check
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
                    size_t remaining = total_size - bytes_received;
                    if (remaining > 0) {
                        char * tmp = (char *)malloc(total_size);
                        memcpy(tmp, full_response, bytes_received);
                        free(full_response);
                        full_response = tmp;
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
        process(full_response, bytes_received);

        // either going to write to a file or add another request to the queue
        // write it to a file
        std::ofstream outfile(request.filename());
        outfile.write(full_response, total_size);
        outfile.close();
    }

    void HTTPClient::process(char * file, size_t len) {
        std::cout << "TODO" << '\n';
    }

    char* HTTPClient::check_redirects(char * get_message){
        char redirect_lead_int = get_message[9];
        char* no_redirects = (char*)"No redirects\n";
        if (redirect_lead_int == '3'){
            //Convert GET message to string for ease of access
            std::string message_copy = get_message;
            std::string curr_line = "";
            std::string redirected_url = "";
            //Parse GET message
            for (int i = 0; i < message_copy.length(); ++i){
                //Found the redirected link URL
                if (curr_line == "Location: "){
                    while (message_copy[i] != '\n'){
                        redirected_url += message_copy[i];
                        i++;
                    }
                    break;
                }
                //Reset parsed line if newline char found
                else if (message_copy[i] == '\n'){
                    curr_line = "";
                }
                //Append GET message to curr_line to scan for 'Location: ' string
                else {
                    curr_line += message_copy[i];
                }
            }
            char* final_url = new char [redirected_url.length() + 1];
            strcpy(final_url, redirected_url.c_str());
            return final_url;
        }
        else {
            return no_redirects;
        }
    }

    int HTTPClient::Socket::setFd(int fd_in) {
        sockfd = fd_in;
        return sockfd;
    }

    int HTTPClient::SecureSocket::setFd(int fd_in) {
        sockfd = fd_in;
        ssl = ::SSL_new(search::HTTPClient::sslContext);
        if (!ssl) {
            // log error
        }
        int sslsock = ::SSL_get_fd(ssl);
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
        return ::recv(sockfd, (void*) buf, len, flags);
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
        } else {
            auto rv = ::SSL_read(ssl, (void*)buf, len);
            // todo error handling
            // https://linux.die.net/man/3/ssl_read
            return rv;
        }
    }

    ssize_t HTTPClient::Socket::close() {
        return ::close(sockfd);
    }
    ssize_t HTTPClient::SecureSocket::close() {
        // TODO error handling
        ::SSL_shutdown(ssl);
        ::SSL_free(ssl);
        return ::close(sockfd);
    }
}
