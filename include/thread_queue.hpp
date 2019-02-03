//
//  thread_queue.hpp
//  engine
//
//  Created by Graham Eger on 2/22/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#ifndef thread_queue_hpp_398
#define thread_queue_hpp_398

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <array>

namespace search {
    template <typename T>
    class ThreadQueue
    {
    public:
        void push(const T &d) {
            std::lock_guard<std::mutex> lock(m);
            q.push(d);
        }
        // returns true if sucessfully popped
        // modifies &d with the value of the popped item
        bool pop(T &d) {
            std::lock_guard<std::mutex> lock(m);
            if (!q.empty()) {
                d = q.front();
                q.pop();
                return true;
            }
            return false;
        }
        size_t size() {
            std::lock_guard<std::mutex> lock(m);
            return q.size();
        }
        bool empty() {
            std::lock_guard<std::mutex> lock(m);
            return q.empty();
        }
    private:
        std::queue<T> q;
        std::mutex m;
        std::condition_variable cv;
    };
}
#endif
