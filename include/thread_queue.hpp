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
#include <deque>
#include <mutex>
#include <condition_variable>
#include <array>
#include <vector>

namespace search {
    template <typename T>
    class ThreadQueue
    {
    public:
        void push(const T &d) {
            std::unique_lock<std::mutex> lock(m);
            q.push_back(d);
            cv.notify_one();
        }

        void push(const std::vector<T> &d) {
            std::unique_lock<std::mutex> lock(m);
            for (auto i : d) {
                q.push_back(i);
            }
            cv.notify_all();
        }

        // returns true if sucessfully popped
        // modifies &d with the value of the popped item
        T pop() {
            std::unique_lock<std::mutex> lock(m);
            // we own the lock
            while (q.empty()) {
                cv.wait(lock);
            }
            // we own the lock
            T temp = q.front();
            q.pop_front();
            return temp;
            // mutex gets released on destruction
        }

        // get all of the stuff on the queue
        // used for EventQueue
        void pop_all(std::vector<T> &vec) {
            std::unique_lock<std::mutex> lock(m);
            // we own the lock
            while (q.empty()) {
                cv.wait(lock);
            }
            // we own the lock
            vec = std::vector<T>(q.begin(), q.end());
            q.clear();
            return;
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
        std::deque<T> q;
        std::mutex m;
        std::condition_variable cv;
    };
}
#endif
