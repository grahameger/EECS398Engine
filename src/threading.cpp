// Created by Graham Eger on 3/28/2019
// Graham Eger added ReadWriteLock on 4/1/2019

#include "threading.h"
namespace threading {
    Mutex::Mutex() {
        m = PTHREAD_MUTEX_INITIALIZER;
    }

    Mutex::~Mutex() {
        pthread_mutex_destroy(&m);
    }

     void Mutex::lock() {
        pthread_mutex_lock(&m);
    }

     void Mutex::unlock() {
        pthread_mutex_unlock(&m);
    }

    ConditionVariable::ConditionVariable() {
        cv = PTHREAD_COND_INITIALIZER;
    }

     ConditionVariable::~ConditionVariable() {
        pthread_cond_destroy(&cv);
    }

     void ConditionVariable::wait(Mutex &mutex) {
        pthread_cond_wait(&cv, &mutex.m);
    }

     void ConditionVariable::signal() {
        pthread_cond_signal(&cv);
    }

     void ConditionVariable::broadcast() {
        pthread_cond_broadcast(&cv);
     }

    Semaphore::Semaphore(size_t count_in) {
        int rv = sem_init(&_s, 0, count_in);
            if (rv != 0) {
                fprintf(stderr, "sem_init() failed with error '%s'\n", strerror(errno));
                exit(1);
        }
    }

    Semaphore::~Semaphore() {
        int rv = sem_destroy(&_s);
        if (rv != 0) {
            fprintf(stderr, "sem_destroy() failed with error '%s'\n", strerror(errno));
        }
    }

    void Semaphore::notify() {
        sem_post(&_s);
    }

    void Semaphore::wait() {
        sem_wait(&_s);
    }

    ReadWriteLock::ReadWriteLock() {
        //pthread_rwlock_init(&lock, nullptr);
        lock = PTHREAD_MUTEX_INITIALIZER;
    }

    ReadWriteLock::~ReadWriteLock() {
        //pthread_rwlock_unlock(&lock);
        //pthread_rwlock_destroy(&lock);
        pthread_mutex_unlock(&lock);
        pthread_mutex_destroy(&lock);
    }

    void ReadWriteLock::readLock() {
        //pthread_rwlock_rdlock(&lock);
        pthread_mutex_lock(&lock);
    }

    void ReadWriteLock::writeLock() {
        //pthread_rwlock_wrlock(&lock);
        pthread_mutex_lock(&lock);
    }

    void ReadWriteLock::unlock() {
        //pthread_rwlock_unlock(&lock);
        pthread_mutex_unlock(&lock);
    }

}