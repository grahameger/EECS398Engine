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
    };
}

#endif