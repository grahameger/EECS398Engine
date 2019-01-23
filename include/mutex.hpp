//
//  mutex.hpp
//  pthread_wrapper
//
//  Created by Graham Eger on 1/16/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#ifndef mutex_hpp_398
#define mutex_hpp_398

#include <stdio.h>
#include <pthread.h>

namespace threading {
   class mutex {
      friend class condition_variable;
      
   public:
      void lock();
      void try_lock();
      void unlock();
      mutex();
      virtual ~mutex();
      
   protected:
      pthread_mutex_t _m;
   };
}

#endif /* mutex_hpp */
