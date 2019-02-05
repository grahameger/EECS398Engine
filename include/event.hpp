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

#include "thread_queue.hpp"
#include <iostream>
#include <thread>

#ifdef __linux__ 
#include <sys/epoll.h>
#else
#include <sys/types.h>
// http://www.manpagez.com/man/2/kevent/
#include <sys/event.h>
#include <sys/time.h>
#endif

namespace search {
    class EventQueue
    {
    public:
        EventQueue();
        ~EventQueue();
        int getSocket();
        void addSocket(int sockfd);
    private:
        static const size_t MAX_CONNECTIONS = 1000;

        void process();

        ThreadQueue<int> readySockets;
        ThreadQueue<int> waitingSockets;
        std::thread t;

        #ifdef __linux__ 
        int epollFd;
        #else
        int kq;
        struct kevent64_s chlist[MAX_CONNECTIONS];
        struct kevent64_s evlist[MAX_CONNECTIONS];
        #endif
    };
}


#endif