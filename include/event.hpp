//
//  event.hpp
//  engine
//
//  Created by Graham Eger on 2/2/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//
// Abstraction for event driven netowrk IO on Linux / Mac / FreeBSD / NetBSD

#ifndef event_hpp_398
#define event_hpp_398

#include <iostream>
#include <utility>
#include <vector>
#include <thread>
// https://medium.com/@copyconstruct/the-method-to-epolls-madness-d9d2d6378642
#include <sys/epoll.h>



namespace search {
    class EventQueue
    {
    public:
        EventQueue();
        ~EventQueue();
        int getSocket();
        std::vector<int> getSockets();
        void addSocket(int sockfd);
    private:
        static const size_t MAX_CONNECTIONS = 1000;
        static const int MAX_EVENTS = 64;
        void process();
        std::thread t;
        int epollFd;
    };
}


#endif