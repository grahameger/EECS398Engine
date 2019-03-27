//
//  thread_queue.hpp
//  engine
//
//  Created by Graham Eger on 2/22/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#pragma once
#ifndef thread_queue_hpp_398
#define thread_queue_hpp_398

#include <iostream>
#include <deque>
#include <array>
#include <vector>
#include <pthread.h>

#include "List.h"

namespace threading {

    template <typename T>
    class ThreadQueue
    {
    public:

        ThreadQueue() {
            m = PTHREAD_MUTEX_INITIALIZER;
            cv = PTHREAD_COND_INITIALIZER;
        }

        void push(const T &d) {
            pthread_mutex_lock(&m);
            q.push_back(d);
            pthread_cond_signal(&cv);
            pthread_mutex_unlock(&m);
        }

        void push(const std::vector<T> &d) {
            pthread_mutex_lock(&m);
            for (auto i : d) {
                q.push_back(i);
            }
            pthread_cond_broadcast(&cv);
            pthread_mutex_unlock(&m);
        }

        // returns true if sucessfully popped
        // modifies &d with the value of the popped item
        T pop() {
            //std::unique_lock<std::mutex> lock(m);
            pthread_mutex_lock(&m);
            // we own the lock
            while (q.empty()) {
                pthread_cond_wait(&cv, &m);
            }
            // we own the lock
            T temp = q.front();
            q.pop_front();
            return temp;
            // mutex gets released on destruction
            pthread_mutex_unlock(&m);
        }

        // get all of the stuff on the queue
        // used for EventQueue
        void pop_all(std::vector<T> &vec) {
            pthread_mutex_lock(&m);
            // we own the lock
            while (q.empty()) {
                pthread_cond_wait(&cv, &m);
            }
            // we own the lock
            vec = std::vector<T>(q.begin(), q.end());
            q.clear();
            pthread_mutex_unlock(&m);
        }

        size_t size() {
            pthread_mutex_lock(&m);
            auto rv = q.size();
            pthread_mutex_unlock(&m);
            return rv; 
        }
        bool empty() {
            pthread_mutex_lock(&m);
            auto rv = q.empty();
            pthread_mutex_unlock(&m);
            return rv;
        }
    private:
        std::deque<T> q;
        pthread_mutex_t m;
        pthread_cond_t cv;
    };
}
#endif
