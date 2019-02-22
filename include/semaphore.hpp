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
#include <semaphore.h>

namespace threading {
    template <size_t N>
    class Semaphore {
    public:
        Semaphore() {
            sem_init(&s, 0, N);
        }
        ~Semaphore() {
            sem_destroy(&s);
        }
        void notify() {
            sem_post(&s);
        }
        void wait() {
            sem_wait(&s);
        }
    private:
        sem_t s;
    };
}

#endif