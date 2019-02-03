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

namespace search {
    class EventQueue
    {
    public:
        EventQueue();
        ~EventQueue();
        int getSocket();
        void addSocket(int sockfd);
    private:
        void process();

        ThreadQueue<int> readySockets;
        ThreadQueue<int> waitingSockets;
        std::thread t;
    };
}


#endif