//
//  event.cpp
//  engine
//
//  Created by Graham Eger on 2/2/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//
// 

#include "event.hpp"
#include <unistd.h>

namespace search {
    EventQueue::EventQueue() {
        epollFd = epoll_create1(0);
    }

    int EventQueue::getSocket() {
        epoll_event event;
        // block no timeout
        int rv = epoll_wait(epollFd, &event, 1, -1);
        return event.data.fd;
        if (rv == -1) {
            // TODO log error
        }
    }

    std::vector<int> EventQueue::getSockets() {
        std::vector<int> socks;
        epoll_event events[MAX_EVENTS];
        // taking MAX_EVENTS and blocking. No timeout.
        int numFds = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        // only allocate once and what we need
        socks.reserve(numFds);
        for (ssize_t i = 0; i < numFds; i++) {
            socks.push_back(events[i].data.fd);
        }
        // won't make a copy of the vector
        return std::move(socks);
    }

    void EventQueue::addSocket(int sockfd) {
        epoll_event ev;
        // watch for read events and closed socket events.
        // using EPOLLONESHOPT to prevent race conditions. Must readd the 
        // file descriptor after each read.               edge triggered
        ev.events = EPOLLIN | EPOLLRDHUP | EPOLLONESHOT | EPOLLET;
        ev.data.fd = sockfd;
        // when sockfd closes it will be automatically removed from
        // the kernel epoll data structure.
        int rv = epoll_ctl(epollFd, EPOLL_CTL_ADD, sockfd, &ev);
        if (rv == -1) {
            // TODO log error
        }
    }

    void EventQueue::removeSocket(int sockfd) {
        close(sockfd);
    }
}

