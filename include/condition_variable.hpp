//
//  condition_variable.hpp
//  pthread_wrapper
//
//  Created by Graham Eger on 1/16/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#ifndef condition_variable_hpp_398
#define condition_variable_hpp_398

#include <stdio.h>
#include <pthread.h>
#include "mutex.hpp"

namespace threading {
   class condition_variable {
   public:
   ~condition_variable();
   condition_variable();
    void wait(mutex &m);
    void signal();
    void broadcast();
   protected:
      pthread_cond_t _c;
   };
}

#endif /* mutex_hpp */
