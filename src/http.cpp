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
#include <dirent.h>
#include <map>
#include "Parser.hpp"
#include "Debug.h"

namespace search {

    HTTPClient::HTTPClient(search::Crawler * crawlerIn) {
        crawler = crawlerIn;
        robots = &threading::Singleton<RobotsTxt>::getInstance();

        const char fileWritesLogName[] = "fileWrites.log";

        struct stat st = {0};
        int oFlags = O_WRONLY | O_APPEND;
        if (stat(fileWritesLogName, &st) != 0) {
            oFlags |= O_CREAT;
        }
        logFd = open(fileWritesLogName, oFlags, 0755);
        if (logFd < 0) {
            fprintf(stderr, "error opening log file");
            exit(1);
        }

        // initialize the page filter
        static const char robotsDir[] = "robots";
        static const char pagesDir[] = "pages";
        DIR * d;
        struct dirent * dir;
        char fullPath[1000];
        d = opendir(robotsDir);
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                if (dir->d_type == DT_REG) {
                    fullPath[0] = '\0';
                    strcat(fullPath, robotsDir);
                    strcat(fullPath, "/");
                    strcat(fullPath, dir->d_name);
                    struct stat st = {0};
                    if (stat(fullPath, &st) == 0) {
                        crawler->numBytes += st.st_size;
                        crawler->numRobots++;
                        auto s = std::string(fullPath);
                        crawler->pageFilter.add(s);
                        // add the file to the robots rules
                    }
                }
            }
        }
        closedir(d);
        d = opendir(pagesDir);
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                if (dir->d_type == DT_REG)  {
                    fullPath[0] = '\0';
                    strcat(fullPath, robotsDir);
                    strcat(fullPath, "/");
                    strcat(fullPath, dir->d_name);
                    
                    struct stat st = {0};
                    if (stat(fullPath, &st) == 0) {
                        crawler->numBytes += st.st_size;
                        crawler->numPages++;
                        auto s = std::string(fullPath);
                        crawler->pageFilter.add(std::string(s));
                        // open the file, parse it, add the links to a set
                    }
                }
            }
        }
        closedir(d);

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

        #ifdef OPENSSL_VERSION_TEXT
        fprintf(stdout, "%s\n", OPENSSL_VERSION_TEXT);
        #endif

        search::HTTPClient::sslContext = SSL_CTX_new(meth);

        // cross platform stuff
        signal(SIGPIPE, SIG_IGN);
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
        for (size_t i = 0; i < 5; i++) {
            if ((rv = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &servinfo)) != 0) {
                continue;
            } else {
                break;
            }
        }
        if (rv == EAI_AGAIN) {
            D(fprintf(stderr, "getaddrinfo timed out 5 times for host '%s' -- %s -- %s\n", host.c_str(), gai_strerror(rv), strerror(errno));)
        } else if (rv != 0) {
            return -1;
            D(fprintf(stderr, "getaddrinfo failed for host '%s' - %s - %s", host.c_str(), gai_strerror(rv), strerror(errno));)
        }


        if ((rv = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &servinfo)) != 0) {
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
                    D(fprintf(stderr, "could not set socket to non-blocking for host '%s', strerror: %s\n", host.c_str(), gai_strerror(rv));)
                    close(sockfd);
                    freeaddrinfo(servinfo);
                    return -1;
                }
            }
            // timeout section of the show
            timeval tm = {constants::TIMEOUTSECONDS, constants::TIMEOUTUSECONDS};
            if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (timeval*)&tm, sizeof(tm)) == -1) {
                D(fprintf(stderr, "setsockopt failed for host '%s', strerror: %s\n", host.c_str(), strerror(errno));)
                close(sockfd);
                freeaddrinfo(servinfo);
                return -1;
            }
            if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (timeval*)&tm, sizeof(tm)) == -1) {
                D(fprintf(stderr, "setsockopt failed for host '%s', strerror: %s\n", host.c_str(), strerror(errno));)
                close(sockfd);
                freeaddrinfo(servinfo);
                return -1;
            }
            // on linux kernels > 4.13 this is effectively a non-blocking
            // connect call with the timeout specified above.
            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd);
                continue;
            }
            break;
        }
        // end of list with no connection
        if (p == nullptr) {  
            D(fprintf(stderr, "unable to connect to host '%s' - %s\n", host.c_str(), strerror(errno));)
            freeaddrinfo(servinfo);
            return -1;
        }
        freeaddrinfo(servinfo);
        return sockfd;
    }

    HTTPClient::~HTTPClient() {
        // close our log file descriptors
        close(logFd);
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
    bool HTTPClient::goodMimeContentType(const char * str, const ssize_t len) {
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
            if (contentType.size() >= 9) {
                std::string_view firstToken = contentType.substr(0,4);
                std::string_view secondToken = contentType.substr(5,4);
                return firstToken == "text" && (secondToken == "html" || secondToken == "plai");
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    bool HTTPClient::response200or300(const char * str, ssize_t len) {
        return len >= 10 ? str[9] == '2' || str[9] == '3' : false;
    }


    // borrows all arguments
    bool HTTPClient::headerChecks(const char * header, const size_t headerLen, const HTTPRequest& req, ssize_t& contentLength) {
        if (!response200or300(header, headerLen)) {
            auto uri = req.uri();
            crawler->killFilter.add(uri);
            D(fprintf(stderr, "[BAD RESPONSE] Non 20X or 30X HTTP response code - %s\n", uri.c_str());)
            return false;
        }
        if (!req.robots() && !goodMimeContentType(header, headerLen) && header[9] != '3') {
            auto uri = req.uri();
            crawler->killFilter.add(uri);
            D(fprintf(stderr, "[BAD CONTENT TYPE] Bad content type for request %s\n", uri.c_str());)
            return false;
        }
        contentLength = getContentLength(std::string_view(header, headerLen));
        if (( size_t )contentLength == CHUNKED) {
            D(fprintf(stderr, "[CHUNKED] received a chunked encoding\n");)
            return false;
        }
        return true;
    }

    void HTTPClient::robotsErrorCheck(const HTTPRequest& request) {
        if (request.robots()) {
            auto filename = request.filename();
            // move all the waiting pages to the readyQueue;
            pthread_mutex_lock(&crawler->waitingForRobotsLock);
            crawler->robotsDomains.insert(request.host);
            auto it = crawler->waitingForRobots.find(request.host);
            if (it != crawler->waitingForRobots.end()) {
                std::set<std::string> readyToCrawl;
                readyToCrawl.insert(it->second.begin(), it->second.end());
                crawler->waitingForRobots.erase(it);
                pthread_mutex_unlock(&crawler->waitingForRobotsLock);
                crawler->readyQueue.push(readyToCrawl.begin(), readyToCrawl.end());
            } else {
                pthread_mutex_unlock(&crawler->waitingForRobotsLock);
            }
            // create an empty file with the filename
            int fd = open(filename.c_str(), O_RDWR | O_CREAT, 0755);
            if (fd != -1) {
                // write the null character to the file
                dprintf(fd, "%c", '\0');
            }
            close(fd);
        }
    }


    void HTTPClient::SubmitURLSync(const std::string &url, size_t redirCount) {
        if (redirCount > REDIRECT_MAX) {
            std::string uri = HTTPRequest(url).uri();
            crawler->killFilter.add(uri);
            D(fprintf(stderr, "[TOO MANY REDIRECTS] %s\n", url.c_str());)
            return;
        }
        HTTPRequest request(url);
        if (request.host == "") {
            D(fprintf(stderr, "[NOHOST] %s\n", url.c_str());)
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
            D(fprintf(stderr, "[NOSCHEME] %s\n", url.c_str());)
            return;
        }
        // open a socket to the host
        int sockfd = getConnToHost(request.host, request.port , true);
        if (sockfd < 0) {
            crawler->killFilter.add(request.uri());
            D(fprintf(stderr, "[CONN FAILED] %s\n", url.c_str());)
            return; 
        }
        ssize_t rv = sock->setFd(sockfd);
        if (rv < 0) {
            // this error message was getting really annoying.
            // fprintf(stderr, "error setting file descriptor for host '%s' : %s\n", url.c_str(), strerror(errno));
            crawler->killFilter.add(request.uri());
            D(fprintf(stderr, "[SET FD FAILED] %s\n", url.c_str());)
            return;
        }

        rv = 0;
        size_t bytesReceived = 0;
        size_t totalSize = 0;
        size_t headerPos = 0;
        size_t headerSize = 0;
        size_t currentBufferSize = 0;
        ssize_t contentLength = -1;
        char * fullResponse = (char*)malloc(constants::BUFFER_SIZE);
        currentBufferSize = constants::BUFFER_SIZE;

        // send request blocking
        const std::string requestStr = request.requestString();
        const std::string uri = request.uri();
        //fprintf(stderr, "uri: %s\n%s\n", uri.c_str(), requestStr.c_str());
        if (sock->send(requestStr.c_str(), requestStr.size()) == -1) {
            D(fprintf(stderr, "[SEND ERROR] %s\n", url.c_str());)
            robotsErrorCheck(request);
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
                D(fprintf(stderr, "[RECV FAILED] %s\n", url.c_str());)
                free(fullResponse);
                robotsErrorCheck(request);
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
                    if (!headerChecks(fullResponse, headerSize, request, contentLength)) {
                        robotsErrorCheck(request);
                        free(fullResponse);
                        return;
                    }
                    if (contentLength != -1) {
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
                            if (rv < 0 || bytesReceived < (size_t)contentLength) {
                                // error reading from socket
                                break;
                            }
                        }
                    } else {
                        // recv until EOF
                        rv = 0;
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
                            D(fprintf(stderr, "[RECV FAILED] %s\n", url.c_str());)
                            robotsErrorCheck(request);
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
            // make a non relative url from this 
            std::string newUrl = resolveRelativeUrl(url.c_str(), redirectUrl);
            free(fullResponse);
            free(redirectUrl);
            redirCount++;
            D(fprintf(stderr, "[REDIRECT] %s\n", newUrl.c_str());)
            return SubmitURLSync(newUrl, redirCount);
        }

        if (!isARobotsRequest) {
            if (containsGzip(fullResponse, headerSize)) {
                D(fprintf(stderr, "[GZIP] %s\n", url.c_str());)
                robotsErrorCheck(request);
                free(fullResponse);
                return;
            } else {
                if (currentBufferSize == bytesReceived) {
                    fullResponse = (char*)realloc(fullResponse, bytesReceived + 1);
                    fullResponse[bytesReceived] = '\0';
                }
                process(fullResponse + headerSize, bytesReceived - headerSize, url);
            }
        }

        // write it to a file
        if (!writeToFile(request, fullResponse, bytesReceived, headerSize)) {
            // don't need to free?
            // definitely need to free
            D(fprintf(stderr, "[FILE WRITE FAILED] %s\n", url.c_str());)
            return;
        }

        if (isARobotsRequest) {
            // TODO: make the data structure itself thread safe in a
            // more optimized way than doing this...
            robots->lock();
            auto filename = request.filename();
            try {
                robots->SubmitRobotsTxt(request.host, filename);
                robots->unlock();
            } catch (...) {
                D(fprintf(stderr, "[ROBOTS SUBMIT FAILED] %s\n", url.c_str());)
                robots->unlock();
            }
            // move all the waiting pages to the readyQueue;
            pthread_mutex_lock(&crawler->waitingForRobotsLock);
            crawler->robotsDomains.insert(request.host);
            auto it = crawler->waitingForRobots.find(request.host);
            std::set<std::string> readyToCrawl;
            if (it != crawler->waitingForRobots.end()) {
                readyToCrawl.insert(it->second.begin(), it->second.end());
                crawler->waitingForRobots.erase(it);
            }
	        // remove the iterator for the host from the map
            pthread_mutex_unlock(&crawler->waitingForRobotsLock);
            crawler->readyQueue.push(readyToCrawl.begin(), readyToCrawl.end());
        }
    }

    // write an HTML file to disk
    // we'll see if this segfaults :)
    bool HTTPClient::writeToFile(const HTTPRequest& req, void * fullRespnose, size_t bytesReceived, size_t headerSize) {
        
        const auto filename = req.filename();
        struct stat st = {0};
        if (stat(filename.c_str(), &st) == 0) {
            free(fullRespnose);
            return false;
        }
        int fd = open(filename.c_str(), O_WRONLY | O_CREAT, 0755);
        for (size_t i = 0; i < 5; ++i) {
            fd = open(filename.c_str(), O_RDWR | O_CREAT, 0755);
            if (fd == -1) {
                // check errno
                switch (errno) {
                    // all of these cases can't be retried, the rest can
                    case EACCES:
                    case EDQUOT:
                    case EINTR:
                    case EINVAL:
                    case EISDIR:
                    case ELOOP:
                    case EMFILE:
                    case ENAMETOOLONG:
                    case ENFILE:
                    free(fullRespnose);
                    return false;
                    default:
                    continue;
                }
            } else {
                break;
            }
        }
        if (fd == -1) {
            dprintf(logFd, "error opening file %s - %s", filename.c_str(), strerror(errno));
            free(fullRespnose);
            return false;
        } else {
            // write the file
            ssize_t bytesWrote = 0;
            const ssize_t bytesToWrite = bytesReceived - headerSize;
            char * filePointer = (char*)fullRespnose + headerSize;
            ssize_t rv = 0;
            while (bytesWrote < bytesToWrite) {
                rv = write(fd, filePointer + bytesWrote, bytesToWrite - bytesWrote);
                if (rv > 0) {
                    bytesWrote += rv;
                } else if (rv == 0) {
                    break;
                } else if (rv < 0) {
                    // destroy the file and return false
                    remove(filename.c_str());
                    close(fd);
                    free(fullRespnose);
                    return false;
                }
            }
            if (bytesWrote != bytesToWrite) {
                // destroy the file and return false
                remove(filename.c_str());
                close(fd);
                free(fullRespnose);
                return false;
            }
            close(fd);
            crawler->numBytes += bytesWrote;
            if (req.robots()) {
                crawler->numRobots++;
            } else {
                crawler->pageFilter.add(filename);
                crawler->numPages++;
            }
            dprintf(logFd, "wrote %s to disk.\n", filename.c_str());
            free(fullRespnose);
            return true;
        }
    }

    bool HTTPClient::containsGzip(char * p, size_t len) {
        std::string_view header(p, len);
        size_t i = header.find("gzip");
        return i != std::string_view::npos;
    }


    // this function would probably fit better in crawler.cpp
    void HTTPClient::process(char * file, size_t len, const std::string& currentUri) {
        crawler->readyQueue.push("");
        const char * baseUri = currentUri.c_str();
        LinkFinder linkFinder(file, len, currentUri, true); // each thread should probably just have they're own one of these
        // TODO: refactor this, probably slow as shit. It's CPU not IO so lower priority.
        std::set<std::string> toPush;
        for (size_t i = 0; i < linkFinder.Document.vector_of_link_anchor.size(); ++i) {
            if (linkFinder.Document.vector_of_link_anchor[i].link_url.Size() < 2083) {
                toPush.insert(resolveRelativeUrl(baseUri, linkFinder.Document.vector_of_link_anchor[i].link_url.CString()));
            }
        }
        fprintf(stdout, "Adding %zu urls to the frontier\n", toPush.size());
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
        std::string timBernersLeeBuffer(base.begin(), base.end());
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
        size_t startIndex = 0;
        while ((segment = path.find("/../", startIndex)) != std::string::npos) {
            std::string tmp = path.substr(0, segment);
            size_t slash = tmp.rfind('/');
            if(segment == 0) {
                startIndex = segment + 4;
            }
            else if (slash == std::string::npos) {
                path = path.substr(segment + 4);
            }
            else if (tmp.substr(0, slash) != "..") {
                path = path.substr(0, slash + 1) + path.substr(segment + 4);
            } else {
                startIndex = segment + 4;
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

    HTTPClient::SecureSocket::SecureSocket() : ssl(nullptr) {}
    HTTPClient::Socket::Socket() : sockfd(-1) {}

    HTTPClient::SecureSocket::~SecureSocket() {
        if (ssl) {
            close();
        }
    }

    HTTPClient::Socket::~Socket() {
        ::close(sockfd);
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
            D(fprintf(stderr, "Error creating new SSL struct\n");)
            return -1;
        }
        int rv = ::SSL_set_fd(ssl, sockfd);
        if (rv != 1) {
            D(fprintf(stderr, "Error in SSL_set_fd with fd: %d\n", sockfd);)
            return -1;
        }
        connect:
        rv = ::SSL_connect(ssl);
        int errnoSSL = SSL_get_error(ssl, rv);
        switch (errnoSSL) {
            case SSL_ERROR_NONE:
                return 0;
            case SSL_ERROR_WANT_CONNECT:
                goto connect;
            case SSL_ERROR_SYSCALL:
                D(fprintf(stderr, "SSL syscall error %s\n", strerror(errno));)
                return -1;
            case SSL_ERROR_ZERO_RETURN:
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
            case SSL_ERROR_WANT_X509_LOOKUP:
            case SSL_ERROR_WANT_ASYNC:
            case SSL_ERROR_WANT_ASYNC_JOB:
            case SSL_ERROR_WANT_CLIENT_HELLO_CB:
            case SSL_ERROR_SSL:
            default:
                D(fprintf(stderr, "%s", "SSL connect other error\n");)
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
            m.lock();
            // ::SSL_shutdown(ssl); this was causing issues
            ::SSL_free(ssl);
            ssl = nullptr;
            m.unlock();
        }
        if (sockfd > 2) {
            int rv = ::close(sockfd);
            sockfd = -1;
            return rv;
        }
        return 0;
    }
    
}
