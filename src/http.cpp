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
#include <zlib.h>
#include <map>
#include "Parser.hpp"

namespace search {

    HTTPClient::HTTPClient(search::Crawler * crawlerIn) {
        crawler = crawlerIn;
        robots = &threading::Singleton<RobotsTxt>::getInstance();

        // this will become a bug if there is ever more than
        // one instance of HTTP client.
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();

        #ifdef LWS_HAVE_TLS_CLIENT_METHOD
        static const SSL_METHOD * meth = TLS_client_method();
        #else
        static const SSL_METHOD * meth = SSLv23_client_method();
        #endif

        search::HTTPClient::sslContext = SSL_CTX_new(meth);

        // cross platform stuff
        signal(SIGPIPE, SIG_IGN);
    }

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
        memset(&hints, 0x0, sizeof hints);
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
                continue;
            }
            if (!blocking) {
                rv = fcntl(sockfd, F_SETFL, O_NONBLOCK);
                if (rv == -1) {
                    fprintf(stderr, "could not set socket to non-blocking for host '%s', strerror: %s\n", host.c_str(), gai_strerror(rv));
                    close(sockfd);
                    freeaddrinfo(servinfo);
                    return -1;
                }
            }
            // timeout section of the show
            timeval tm = {constants::TIMEOUTSECONDS, constants::TIMEOUTUSECONDS};
            if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (timeval*)&tm, sizeof(tm)) == -1) {
                fprintf(stderr, "setsockopt failed for host '%s', strerror: %s\n", host.c_str(), strerror(errno));
                close(sockfd);
                freeaddrinfo(servinfo);
                return -1;
            }

            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd);
                continue;
            }
            break;
        }
        // end of list with no connection
        if (p == nullptr) {  
            fprintf(stderr, "unable to connect to host '%s' - %s\n", host.c_str(), strerror(errno));
            freeaddrinfo(servinfo);
            return -1;
        }
        freeaddrinfo(servinfo);
        return sockfd;
    }

    HTTPClient::~HTTPClient() {
        // SSL Stuff, shouldn't run until all the threads return!
        destroySSL();
    }

    void HTTPClient::destroySSL() {
        ERR_free_strings();
        EVP_cleanup();
    }

    static size_t caseInsensitiveStringFind(const std::string_view& str, const std::string& toFind) {
        auto it = std::search(str.begin(), str.end(), toFind.begin(), toFind.end(), 
                                [](char a, char b) { return std::toupper(a) == std::toupper(b);});
        if (it != str.end()) {
            return it - str.begin();
        } else {
            return std::string_view::npos; 
        }
    }

    static ssize_t getContentLength(std::string_view response) {

        // we need to check for "Transfer Encoding: Chunked" first
        // if there's no transfer encoding: chunked then we can look for content-length
        // https://greenbytes.de/tech/webdav/rfc7230.html#header.content-length
        static const std::string TRANSFER_ENCODING_STR = "chunked";
        if (caseInsensitiveStringFind(response, TRANSFER_ENCODING_STR) != std::basic_string_view<char>::npos) {
            return HTTPClient::CHUNKED;
        }

        static const std::string CONTENT_LENGTH_STR = "Content-Length: ";
        size_t contentLenStart = caseInsensitiveStringFind(response, CONTENT_LENGTH_STR);
        // size_t contentLenStart = response.find(CONTENT_LENGTH_STR);
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

    // Returns false if there is no Content-Type, or if the Content-Type
    // is not either "text/html" or "text/plain"
    // tested good
    bool HTTPClient::goodMimeContentType(char * str, ssize_t len) {
        // construct a string_view from our pointer and length
        // find "Content-Type" using the case insensitive find function
        // construct a string_view of the mime-type indicated in the header
        // see if the first part is "text", if it is, check that the second part is "plain", or "html", nothing else
        // otherwise return true and we'll throw this whole request out and move on
        // if it's a robots.txt 

        // string_view doesn't make any copies, it's essentially a pascal string
        const std::string_view header(str, len);
        const size_t contentTypeStart = caseInsensitiveStringFind(header, constants::contentTypeString);
        if (contentTypeStart != std::string_view::npos) {
            const size_t firstTokenStart = contentTypeStart + constants::contentTypeString.size();
            // find the next whitespace character or a colon
            // also didn't make a copy of the string
            const std::string_view contentType = header.substr(firstTokenStart,
                                             header.find_first_of(" \f\n\r\t\v;", firstTokenStart));
            auto firstToken = contentType.substr(0,4);
            auto secondToken = contentType.substr(5,4);
            return contentType.size() >= 9 && firstToken == "text" && (secondToken == "html" || secondToken == "plai");
        } else {
            return false;
        }
    }


    bool HTTPClient::response200or300(char * str, ssize_t len) {
        return len >= 10 ? str[9] == '2' || str[9] == 3 : false;
    }


    void HTTPClient::SubmitURLSync(const std::string &url, size_t redirCount) {
        if (redirCount > REDIRECT_MAX) {
            fprintf(stderr, "Too many redirects ending in host %s\n", url.c_str());
            std::string uri = HTTPRequest(url).uri();
            crawler->killFilter.add(uri);
            return;
        }
        HTTPRequest request(url);
        if (request.host == "") {
            return;
        }
        std::unique_ptr<Socket> sock;
        //request.method = constants::getMethod;
        if (request.scheme == constants::httpsStr) {
            request.port = 443;
            sock = std::unique_ptr<SecureSocket>(new SecureSocket);
        }
        else if (request.scheme == constants::httpStr) {
            request.port = 80;
            sock = std::unique_ptr<Socket>(new Socket);
        }
        else {
            return;
        }
        // open a socket to the host
        int sockfd = getConnToHost(request.host, request.port , true);
        if (sockfd < 0) {
            crawler->killFilter.add(request.uri());
            return; 
        }
        ssize_t rv = sock->setFd(sockfd);
        if (rv < 0) {
            fprintf(stderr, "error setting file descriptor for host '%s' : %s\n", url.c_str(), strerror(errno));
            crawler->killFilter.add(request.uri());
            return;
        }

        rv = 0;
        int bytesReceived = 0;
        size_t totalSize = 0;
        size_t headerPos = 0;
        size_t headerSize = 0;
        size_t currentBufferSize = 0;
        ssize_t contentLength = -1;
        char * fullResponse = (char*)malloc(constants::BUFFER_SIZE);
        currentBufferSize = constants::BUFFER_SIZE;

        // send request blocking
        const std::string requestStr = request.requestString();
        if (sock->send(requestStr.c_str(), requestStr.size()) == -1) {
            fprintf(stderr, "error sending request to host for url '%s'\n", url.c_str());
            crawler->killFilter.add(request.uri());
            return;
        }

        // dynamic buffering
        // every time recv returns we'll look for "Content-Length", length of the body
        // when we get t he length of the body then we can have a hard coded size to check for
        // get the size of the header by searching for /r/n/r/n
        bool isARobotsRequest = request.robots();
        while (true) {
            rv = sock->recv(fullResponse + bytesReceived, constants::BUFFER_SIZE - bytesReceived, 0);
            if (rv < 0) {
                // error check
                fprintf(stderr, "recv returned an error for url '%s'\n", url.c_str());
                free(fullResponse);
                crawler->killFilter.add(request.uri());
                return;
            } else if (rv == 0) {
                // handle EOF
                break;
                // might need to do more?
            } else if (contentLength == -1) {
                // check if the header has been downloaded
                bytesReceived += rv;
                std::string_view view(fullResponse, bytesReceived);
                headerPos = view.find("\r\n\r\n");
                if (headerPos != std::string_view::npos) {
                    headerSize = headerPos + 4;
                    // once we have the full header check the content type
                    if (!isARobotsRequest && response200or300(fullResponse, headerSize) && !goodMimeContentType(fullResponse, headerSize)) {
                        auto uri = request.uri();
                        crawler->killFilter.add(request.uri());
                        fprintf(stderr, "Bad content type for url %s\n", uri.c_str());
                        free(fullResponse);
                        return;
                    }
                    // search for "Content-Length"
                    contentLength = getContentLength(std::string_view(fullResponse, bytesReceived));
                    if (contentLength == CHUNKED) {
                        fprintf(stderr, "received a chunked encoding\n");
                        return;
                    }
                    if (contentLength != (size_t)-1) {
                        totalSize = headerSize + contentLength;
                        ssize_t remaining = totalSize - bytesReceived;
                        if (remaining > 0) {
                            fullResponse = (char *)realloc(fullResponse, totalSize);
                            currentBufferSize = totalSize;
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
                    } else {
                        // recv until EOF
                        rv = 0;
                        size_t currentBufferSize = constants::BUFFER_SIZE;
                        do {
                            bytesReceived += rv;
                            if (bytesReceived == currentBufferSize) {
                                // do a realloc and double the buffer size
                                fullResponse = (char*)realloc(fullResponse, currentBufferSize * 2);
                                currentBufferSize *= 2;
                            }
                            // we'll do a receive of the total amount of memory remaining in the buffer
                            // we're also not going to do a MSG_WAITALL because this is most likely
                            // going to loop, but it IS still a BLOCKING operation
                            rv = sock->recv(fullResponse + bytesReceived, currentBufferSize - bytesReceived, 0);
                        } while (rv > 0);
                        if (rv < 0) {
                            free(fullResponse);
                            fprintf(stderr, "recv returned an error for url '%s'\n", url.c_str());
                            crawler->killFilter.add(request.uri());
                            return;
                        }
                        break;
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
            // make a non relative url from this 
            std::string newUrl = resolveRelativeUrl(url.c_str(), redirectUrl);
            free(redirectUrl);
            return SubmitURLSync(newUrl, ++redirCount);
        }
        if (!isARobotsRequest) {
            if (containsGzip(fullResponse, headerSize)) {
                free(fullResponse);
                return;
            } else {
                process(fullResponse + headerSize, bytesReceived - headerSize, url);
            }
        }

        // either going to write to a file or add another request to the queue
        // write it to a file
        std::string filename = request.filename();
        if (isARobotsRequest) {
            std::ofstream outfile(filename);
            outfile.write(fullResponse + headerSize, bytesReceived - headerSize);
            outfile.close();
            free(fullResponse);
            // fprintf(stdout, "wrote: %s to disk.\n", filename.c_str());
            // TODO: make the data structure itself thread safe in a
            // more optimized way than doing this...
            robots->lock();
            robots->SubmitRobotsTxt(request.host, filename);
            robots->unlock();
            // move all the waiting pages to the readyQueue;
            pthread_mutex_lock(&crawler->waitingForRobotsLock);
            auto it = crawler->waitingForRobots.find(request.host);
            std::set<std::string> readyToCrawl;
            if (it != crawler->waitingForRobots.end()) {
                readyToCrawl.insert(it->second.begin(), it->second.end());
                crawler->waitingForRobots.erase(it);
            }
	        // remove the iterator for the host from the map
            pthread_mutex_unlock(&crawler->waitingForRobotsLock);
            crawler->readyQueue.push(readyToCrawl.begin(), readyToCrawl.end());
        } else {
            // let's try out the file abstraction
            File(filename.c_str(), fullResponse + headerSize, bytesReceived - headerSize);
        }
    }

    bool HTTPClient::containsGzip(char * p, size_t len) {
        std::string_view header(p, len);
        size_t i = header.find("gzip");
        return i != std::string_view::npos;
    }


    // this function would probably fit better in crawler.cpp
    void HTTPClient::process(char * file, size_t len, const std::string& currentUri) {
        const char * baseUri = currentUri.c_str();
        LinkFinder linkFinder; // each thread should probably just have they're own one of these
        linkFinder.parse(file, len);
        // TODO: refactor this, probably slow as shit. It's CPU not IO so lower priority.
        std::set<std::string> toPush;
        for (size_t i = 0; i < linkFinder.Link_vector.size(); ++i) {
            toPush.insert(resolveRelativeUrl(baseUri, linkFinder.Link_vector[i].CString()));
        }
        crawler->readyQueue.push(toPush.begin(), toPush.end());
    }

    std::string HTTPClient::resolveRelativeUrl(const char * baseUri, const char * newUri) {
        // RFC 2396 Section 5.2 Resolving Relative References to Absolute Form
        // 1) Start 
        // Parse the URI reference into the potential four components and fragment identifier
        HTTPRequest newParsed = HTTPRequest(newUri);
        HTTPRequest baseParsed = HTTPRequest(baseUri);

        // 2)                                                   
        if (newParsed.path == "" && 
            newParsed.scheme == "" &&
            newParsed.host == "" && // authority == host + port
            newParsed.query == "") {
            
            // it is a reference to the current document and we are done
            // we should clear the query and fragment
            baseParsed.fragment = "";
            baseParsed.query = "";
            return baseParsed.uri();
        }

        // 3)
        if (newParsed.scheme != "") {
            // The URL is an absolute URL and we are done, return the 
            // parsed version 
            return std::string(newUri);
        }

        // 4) if host+port is defined then reference is a network path and skip to step 7
        // In our crawler we're not going to crawl network paths outside of :80 or :443
        // so we're just going to return the base URL and let the caller handle it
        if (newParsed.host != "") {
            // skip to step 7
            return std::string(baseUri);
        }

        // 5) if path begins with '/' then the reference is an absolute path, skip to step 7
        if (newParsed.path.begin() != newParsed.path.end() && newParsed.path[0] == '/') {
            // copy the host over and skip to step 7
            newParsed.scheme = baseParsed.scheme;
            newParsed.host = baseParsed.host;
            return newParsed.uri();
        }
        
        // 6) actually resolve the relative-path reference 
        //    the relative path needs to be merged with the base 
        // "Although there are many ways to to this, 
        // we will describe a simple method using a separate string buffer"
        // - Tim Berners Lee
        
        // a) all but the last segment of the base URI's path is copied to the buffer
        // In other words, any characters after the last (right-most) slash character,
        // if any, are excluded.
        std::string base(baseUri);
         // remove everything after the last / in the base Uri path unless the last thing is a slash
        base.erase(base.find_last_of('/') + 1, std::string::npos);
        std::string timBernersLeeBuffer = std::string(base.begin(), base.end());
        // Is it PascalCase because it's a man's name or camelCase because it's local????
        
        // b) The reference's path component is appended to the buffer string
        timBernersLeeBuffer += newParsed.path;

        // everything else is just path normalization so we can parse
        // here and then do the remaining portions on the path string
        HTTPRequest requestToNormalize = HTTPRequest(timBernersLeeBuffer);
        std::string& path = requestToNormalize.path;

        // c) All occurrences of "./", where "." is a complete path segment,
        // are removed from the buffer string.
        for (size_t pos = path.find("/./");
         pos != std::string::npos;
         pos = path.find("/./")) 
        {
            // remove "./", not the first slash
            path.erase(pos + 1, 2);
        }
        
        // d) if the buffer ends with a "." as a complete path segment, 
        // that "." is removed
        if (path.back() == '.' && *(path.rbegin()++) == '/') {
            path.pop_back();
        }

        // e) All occurrences of "<segment>/../", where <segment> is a
        //    complete path segment not equal to "..", are removed from the
        //    buffer string.  Removal of these path segments is performed
        //    iteratively, removing the leftmost matching pattern on each
        //    iteration, until no matching pattern remains.
        // f) If the buffer string ends with "<segment>/..", where <segment>
        //    is a complete path segment not equal to "..", that
        //    "<segment>/.." is removed.
        size_t segment;
        while ((segment = path.find("/../")) != std::string::npos) {
            std::string tmp = path.substr(0, segment);
            size_t slash = tmp.rfind('/');
            if (slash == std::string::npos) {
                continue;
            }
            if (tmp.substr(0, slash) != "..") {
                path = path.substr(0, slash + 1) + path.substr(segment + 4);
            }
        }
        // g) If the resulting buffer string still begins with one or more
        //  complete path segments of "..", then the reference is
        //  considered to be in error.  Implementations may handle this
        //  error by retaining these components in the resolved path (i.e.,
        //  treating them as part of the final URI), by removing them from
        //  the resolved path (i.e., discarding relative levels above the
        //  root), or by avoiding traversal of the reference.
        while (path.size() >= 3 && path[0] == '/' && path[1] == '.' && path[2] == '.') {
            // remove the first three characters of the URI.
            path.erase(0, 3);
        }
        return requestToNormalize.uri();
    }

    char * HTTPClient::checkRedirectsHelper(const char * getMessage, const size_t len) {
        if (len < 10 || !getMessage) {
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
            // TODO: replace this with the new string class
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
            fprintf(stderr, "Error creating new SSL struct\n");
            fflush(stderr);
            return -1;
        }
        int rv = ::SSL_set_fd(ssl, sockfd);
        if (rv != 1) {
            fprintf(stderr, "Error in SSL_set_fd with fd: %d\n", sockfd);
            fflush(stderr);
            return -1;
        }
        rv = ::SSL_connect(ssl);
        if (rv <= 0) {
            fprintf(stderr, "Error creating SSL connection\n");
            fflush(stderr);
            return -1;
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
            // ::SSL_shutdown(ssl); this was causing issues
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
