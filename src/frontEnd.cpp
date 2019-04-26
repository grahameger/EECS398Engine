#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include <thread>
#include <cstdlib>
#include <algorithm>
#include <cassert>
#include <memory>
#include <array>
#include <unordered_map>
#include <condition_variable>
#include <mutex>
#include <sstream>
#include <queue>
#include <cmath>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <list>
#include <deque>
#include <string_view>

#include "frontEnd.h"



FrontEnd::FrontEnd(Index * index_in) {
    index = index_in;
}

void FrontEnd::acceptStub() {
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "socket error %s", strerror(errno));
        exit(1);
    }

    int newFd;
    int rv;
    int optval = 1;
    int sockopt_ret;
    sockopt_ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, size(optval));
    if (sockopt_ret != 0) {
        fprintf(stderr, "socket error %s", strerror(errno));
        exit(1);
    }

    // now do the bind stuff
    struct sockaddr_in server;
    bzero((char *) &server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(80);
    if (bind(sockfd, reinterpret_cast<sockaddr *>(&server), sizeof(server)) == -1) {
        fprintf(stderr, "bind error %s", strerror(errno));
        assert(0);
    }
    rv = listen(sockfd, 10);
    if (rv == -1) {
        fprintf(stderr, "listen error - %s", strerror(errno));
    }

    std::cout << "\nlistening\n";

    while (true) {
        newFd = accept(sockfd, nullptr, nullptr);
        if (newFd == -1 ){
            fprintf(stderr, "accept error %s", strerror(errno));
            continue;
        }
        std::thread newThread(FrontEnd::stub, newFd);
    }
}

void FrontEnd::stub(int fd) {
    // http 1.0 only
    char * buf = (char*)calloc(16834, sizeof(char));
    size_t bytes = 0;
    size_t bytesReceived = 0;
    ssize_t rv = -1;
    
    size_t headerSize = -1;
    while ((rv = recv(fd, buf, 16834, 0)) > 0) {
        // check to see if we have the sentinel value
        bytesReceived += rv;
        std::string_view downloaded(buf, bytesReceived);
        size_t pos = downloaded.find("\r\n\r\n");
        if (pos != std::string_view::npos) {
            continue;
        } else {
            headerSize = pos + 4;
            break;
        }
    }
    ssize_t startOfPath = -1;
    ssize_t endOfPath = -1; 
    if (headerSize != -1) {
        // parse for the path
        // find the first space
        for (size_t i = 0; i < bytesReceived; ++i) {
            // find the first space character
            if (buf[i] = ' ') {
                startOfPath = i + 1;
                break;
            }
        }
        for (size_t i = startOfPath; i < bytesReceived; ++i) {
            // find the first space character
            if (buf[i] = ' ') {
                endOfPath = i;
                break;
            }
        }
        Ranker ranker;
        std::string path = std::string_view(buf + startOfPath, endOfPath - startOfPath);
        Parser parser(path);
        ISR * expr = parser.Parse();
        if (expr && parser.fullParsed()) {
            Rank(expr);
        }
    }
    rv = send(fd, httpResponse, httpResponse.size());
}