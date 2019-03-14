//
//  semaphore.hpp
//  pthread_wrapper
//
//  Created by Graham Eger on 2/14/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#ifndef semaphore_hpp_398
#define semaphore_hpp_398

#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>

namespace threading {
    // class Semaphore {
    // public:
    //     Semaphore(size_t count_in) {
    //         count = count_in;
    //         pthread_mutex_init(&m, nullptr);
    //         pthread_cond_init(&cv, nullptr);
    //     }
    //     ~Semaphore() {
    //         pthread_mutex_destroy(&m);
    //         pthread_cond_destroy(&cv);
    //     }
    //     inline void notify() {
    //         pthread_mutex_lock(&m);
    //         count++;
    //         pthread_mutex_unlock(&m);
    //         pthread_cond_signal(&cv);
    //     }
    //     inline void wait() {
    //         pthread_mutex_lock(&m);
    //         while (count == 0) {
    //             pthread_cond_wait(&cv, &m);
    //         }
    //         count--;
    //         pthread_mutex_unlock(&m);
    //     }
    // private:
    //     size_t count;
    //     pthread_mutex_t m;
    //     pthread_cond_t  cv;
    // };
    class Semaphore {
    public:
        Semaphore(size_t count_in) {
            int rv = sem_init(&_s, 0, count_in);
            if (rv != 0) {
                fprintf(stderr, "sem_init() failed with error '%s'", strerror(errno));
                exit(1);
            }
        }
        ~Semaphore() {
            int rv = sem_destroy(&_s);
            if (rv != 0) {
                fprintf(stderr, "sem_destroy() failed with error '%s'", strerror(errno));
            }
        }
        void notify() {
            sem_post(&_s);
        }
        void wait() {
            sem_wait(&_s);
        }
    private:
        sem_t _s;
    }
}

#endif