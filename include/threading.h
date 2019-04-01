// Created by Graham Eger on 3/28/2019
// Graham Eger added ReadWriteLock on 4/1/2019

#pragma once
#ifndef THREADING_H_398
#define THREADING_H_398

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <deque>
#include <vector>

namespace threading {
    class Mutex  {
    public:
        Mutex();
        ~Mutex();
        void lock();
        void unlock();
    private:
        friend class ConditionVariable;
        Mutex(const Mutex&) = delete;
        Mutex& operator=(const Mutex&) = delete;
        pthread_mutex_t m;
    };

    class ConditionVariable {
    public:
        ConditionVariable();
        ~ConditionVariable();

        ConditionVariable(const ConditionVariable&) = delete;
        ConditionVariable& operator=(const ConditionVariable&) = delete;

        void wait(Mutex& mutex);
        void signal();
        void broadcast();

    private:
        pthread_cond_t cv;
    };

    template <typename T>
    class Singleton {
    public:
        static T& getInstance() {
            static T instance;
            return instance;
        }
    private:
        Singleton() {}
    public:
        Singleton(Singleton const&) = delete;
        void operator=(Singleton const&) = delete;
    };


    class Semaphore {
    public:
        Semaphore(size_t count_in);
        ~Semaphore();
        void notify();
        void wait();
    private:
        Semaphore(const Semaphore &) = delete;
        Semaphore& operator=(const Semaphore& ) = delete;
        sem_t _s;
    };

    template <typename T>
    class ThreadQueue {
    public:
        ThreadQueue() {}
        ~ThreadQueue() {}

        void push(const T &d);
        void push(const std::vector<T> &d);
        T pop();
        void popAll(std::vector<T> &d);

        size_t size();
        bool empty();
    private:
        ThreadQueue(const ThreadQueue<T> &) = delete;
        ThreadQueue& operator=(const ThreadQueue<T>& ) = delete;

        std::deque<T> q;

        Mutex m;
        ConditionVariable cv;
    };

    class ReadWriteLock {
        pthread_rwlock_t lock;
    public:
        ReadWriteLock();
        ~ReadWriteLock();
        void readLock();
        void writeLock();
        void unlock();
    };

    template <typename T> void ThreadQueue<T>::push(const T &d) {
        m.lock();
        q.push_back(d);
        cv.signal();
        m.unlock();
    }

    template <typename T> void ThreadQueue<T>::push(const std::vector<T> &d) {
        m.lock();
        for (auto &i : d) {
            q.push_back(i);
        }
        cv.signal();
        m.unlock();
    } 

    template <typename T> T ThreadQueue<T>::pop() {
        m.lock();
        while (q.empty()) {
            cv.wait(m);
        }
        T temp = q.front();
        q.pop_front();
        m.unlock();
        return temp;
    }

    template <typename T> void ThreadQueue<T>::popAll(std::vector<T> &d) {
        m.lock();
        while (q.empty()) {
            cv.wait(m);
        }
        // we own the lock
        d = std::vector<T>(q.begin(), q.end());
        q.clear();
        m.unlock();
    }

    template <typename T> size_t ThreadQueue<T>::size() {
        m.lock();
        auto rv = q.size();
        m.unlock();
        return rv;
    }

    template <typename T> bool ThreadQueue<T>::empty() {
        m.lock();
        auto rv = q.empty();
        m.unlock();
        return rv;
    }
}

#endif
