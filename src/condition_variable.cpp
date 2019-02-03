//
//  condition_variable.cpp
//  pthread_wrapper
//
//  Created by Graham Eger on 1/23/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#include "condition_variable.hpp"

namespace threading {
    
    condition_variable::condition_variable() {
        int rc = pthread_cond_init(&_c, nullptr);
        if (rc != 0) {
            throw;
            // TODO custom exception class
        }
    }

    condition_variable::~condition_variable() {
        int rc = pthread_cond_destroy(&_c);
        if (rc != 0) {
            throw; 
        }
    }

    void condition_variable::wait(mutex &m) {
        pthread_cond_wait(&_c, &m._m);
    }

    void condition_variable::signal() {
        pthread_cond_signal(&_c);
    }

    void condition_variable::broadcast() {
        pthread_cond_broadcast(&_c);
    }
}