// Created by Graham Eger on 3/28/2019
// Graham Eger added ReadWriteLock on 4/1/2019
// Graham Eger added a templated push function for containers on 04/08/2019

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
#include <random>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <cstdio>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace threading {
    struct Mutex  {
    public:
        Mutex();
        ~Mutex();
        void lock();
        void unlock();
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
        ThreadQueue();
        ~ThreadQueue();

        void push(const T &d);
        void push(const std::vector<T> &d);

        template <typename Iterator>
        void push(Iterator start, Iterator end);
        
        void push();

        void write();

        T pop();
        T popRandom();
        void popAll(std::vector<T> &d);

        size_t size();
        bool empty();
        static const size_t maxSize = 5000000;
    private:
        ThreadQueue(const ThreadQueue<T> &) = delete;
        ThreadQueue& operator=(const ThreadQueue<T>& ) = delete;

        std::deque<T> q;

        void loadFromOverflow();

        Mutex m;
        ConditionVariable cvPop;
        static constexpr const char overflowFilename[] = "overflow.urls";
        int overflowFd;
    };

    class ReadWriteLock {
        // pthread_mutex_t lock;
        pthread_rwlock_t lock;
    public:
        ReadWriteLock();
        ~ReadWriteLock();
        void readLock();
        void writeLock();
        void unlock();
    };

    template <typename T> ThreadQueue<T>::ThreadQueue() {
        // check if the file exists already or not
        int oFlags = O_RDWR | O_APPEND;
        struct stat st = {};
        if (stat(overflowFilename, &st) != 0) {
            // file already exits
            oFlags |= O_CREAT;
        }
        int rv = open(overflowFilename, oFlags, 0755);
        if (rv < 0) {
            fprintf(stderr, "error opening overflow file - %s\n", strerror(errno));
            exit(1);
        } else {
            overflowFd = rv;
        }
    }

    template <typename T> ThreadQueue<T>::~ThreadQueue() {
        close(overflowFd);
    }

    template <typename T> void ThreadQueue<T>::push(const T &d) {
        m.lock();
        if (q.size() > maxSize) {
            // write to the overflow file
            std::stringstream ss;
            ss << d;
            auto s = ss.str();
            dprintf(overflowFd, "%s", s.c_str());
            cvPop.signal();
            m.unlock();
            return;
        }
        q.push_back(d);
        cvPop.signal();
        m.unlock();
    }

    template <typename T> void ThreadQueue<T>::push(const std::vector<T> &d) {
        m.lock();
        if (q.size() > maxSize) {
            // write to the overflow file
            std::stringstream ss;
            for (auto i : d) {
                ss << i << '\n';
            }
            auto s = ss.str();
            dprintf(overflowFd, "%s", s.c_str());
            cvPop.signal();
            m.unlock();
            return;
        }
        for (auto &i : d) {
                q.push_back(i);
        }
        cvPop.signal();
        m.unlock();
    } 


    template <typename T>
    template <typename Iterator>
    void ThreadQueue<T>::push(Iterator start, Iterator end) {
        m.lock();
        if (q.size() > maxSize) {
            // write to the overflow file
            std::stringstream ss;
            for (auto it = start; it != end; it++) {
                ss << *it << '\n';
            }
            auto s = ss.str();
            dprintf(overflowFd, "%s", s.c_str()); 
            cvPop.signal();
            m.unlock();
            return;
        }
        for (auto it = start; it != end; it++) {
            q.push_back(*it);
        }
        cvPop.signal();
        m.unlock();
    }

    // if somehow our queue goes to 0, this should introduce some randomness
    template <typename T>
    void ThreadQueue<T>::loadFromOverflow() {
        m.lock();
        size_t numBytes = 8192; 
        char * line = (char *) malloc (numBytes + 1);
        FILE * queue = fdopen(overflowFd, "r");
        while (getline(&line, &numBytes, queue) > 0) {
            // push the line to the 
            q.push_back(line);
        }
        m.unlock();
    }

    template <typename T> T ThreadQueue<T>::pop() {
        m.lock();
        while (q.empty()) {
            loadFromOverflow();
            if (q.empty()) {
                cvPop.wait(m);
            }
        }
        T temp = q.front();
        q.pop_front();
        m.unlock();
        return temp;
    }

    template <typename T> void ThreadQueue<T>::popAll(std::vector<T> &d) {
        m.lock();
        while (q.empty()) {
            loadFromOverflow();
            if (q.empty()) {
                cvPop.wait(m);
            }
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

    template <typename T> void ThreadQueue<T>::write() {
        static const std::string fileName = "queue.urls";
        static const std::string fileNameNew = "queue.urls.swap";
        static const std::string fileNameOld = "queue.urls.old";
        std::ofstream newFile(fileNameNew);
        m.lock();
        // make a copy of the deque
        std::deque<T> temp = q;
        m.unlock();
        // write it out while not holding the lock
        for (auto i : temp) {
            newFile << i << '\n';
        }
        std::rename(fileName.c_str(), fileNameOld.c_str());
        std::rename(fileNameNew.c_str(), fileName.c_str());
        // atomic write done 
    }
}

#endif
