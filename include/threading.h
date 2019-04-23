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
#include <algorithm>
#include <sstream>
#include <string>
#include <unistd.h>
#include <cstdio>
#include <set>
#include <cassert>
#include <array>

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
        void pushForce(const T& d);
        void push(const std::vector<T> &d);

        template <typename Iterator>
        void push(Iterator start, Iterator end);
        
        void push();

        void write();

        T pop();
        std::vector<T> popVec();
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
        int rv = open(overflowFilename, oFlags, 0766);
        if (rv < 0) {
            fprintf(stderr, "error opening overflow file - %s\n", strerror(errno));
            exit(1);
        } else {
            overflowFd = rv;
        }
        // read the overflow file in and reset the size to 0
        FILE * overFlowFile = fdopen(overflowFd, "r+");
        std::set<std::string> deDuplicator;
        size_t lineLen;
        char * line = nullptr;
        if (overFlowFile) {
            while ((rv = getline(&line, &lineLen, overFlowFile)) != -1) {
                deDuplicator.insert(line);
                free(line); line = nullptr;
            }
        }
        // add the things from the deDuplicator to the queue
        for (auto i : deDuplicator) {
            i.erase(std::remove_if(i.begin(), i.end(), isspace), i.end());
            q.push_back(i);
        }
    }

    template <typename T> ThreadQueue<T>::~ThreadQueue() {
        close(overflowFd);
    }

    template <typename T> void ThreadQueue<T>::push(const T &d) {
        m.lock();
        if (d == T()) {
            cvPop.signal();
            m.unlock();
            return;
        }
        if (q.size() > maxSize) {
            // write to the overflow file
            std::stringstream ss;
            ss << d;
            auto s = ss.str();
            dprintf(overflowFd, "%s\n", s.c_str());
            cvPop.signal();
            m.unlock();
            return;
        }
        q.push_back(d);
        cvPop.signal();
        m.unlock();
    }

    template <typename T> void ThreadQueue<T>::pushForce(const T &d) {
        m.lock();
        if (d == T()) {
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
    // DO NOT LOCK IN THIS FUNCTION
    template <typename T>
    void ThreadQueue<T>::loadFromOverflow() {  
        char * line = NULL;
        size_t len = 0;
        ssize_t rv = 0;
        FILE * queue = fdopen(overflowFd, "r");
        if (!queue) {
            fprintf(stderr, "error opening overflow file - %s\n", strerror(errno));
        }
        while ((rv = getline(&line, &len, queue)) != -1) {
            // push the line to the 
            q.push_back(line); // line is statically allocated?
            free(line);
        }
    }


    template <typename T> T ThreadQueue<T>::pop() {
        m.lock();
        while (q.empty()) {
            loadFromOverflow();
            while (q.empty()) {
                cvPop.wait(m);
            }
        }
        // get a random element 
        static thread_local std::random_device rd;
        static thread_local std::default_random_engine generator(rd());
        static thread_local std::uniform_int_distribution<long long unsigned> distribution(0,0xFFFFFFFFFFFFFFFF);
        size_t random = distribution(generator);
        auto it = q.begin();
        if (q.size() > 1) {
            std::advance( it, random % q.size());
        }
        T temp = *it;
        q.erase(it);
        m.unlock();
        return temp;
    }

    template <typename T> std::vector<T> ThreadQueue<T>::popVec() {
        m.lock();
        while (q.empty()) {
            loadFromOverflow();
            while (q.empty()) {
                cvPop.wait(m);
            }
        }
        std::vector<T> rv;
        if (q.size() >= 50) {
            rv = std::vector<T>(q.begin(), q.begin() + 50);
        } else {
            rv = std::vector<T>(q.begin(), q.end());
        }
        for (size_t i = 0; i < rv.size(); i++) {
            q.pop_front();
        }
        m.unlock();
        return rv;
    }

    template <typename T> void ThreadQueue<T>::popAll(std::vector<T> &d) {
        m.lock();
        while (q.empty()) {
            loadFromOverflow();
            while (q.empty()) {
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



        // std::ofstream
    }
}

#endif
