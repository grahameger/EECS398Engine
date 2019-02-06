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
        t = std::thread(&EventQueue::process, this);
        #ifdef __linux__
        epollFd = epoll_create1(0);
        #else
        kq = kqueue();
        if (kq == -1) {
            // TODO: log error
        }
        #endif
    }

    void EventQueue::process() {
        std::vector<int> toAdd;
        while (true) {
            waitingSockets.pop_all(toAdd);
            // we have a list of sockets that we want to wait on
            #ifdef __linux__
            #else
            #endif
        }
    }

    int EventQueue::getSocket() {
        return readySockets.pop();
    }

    void EventQueue::addSocket(int sockfd) {
        waitingSockets.push(sockfd);
    }
}

