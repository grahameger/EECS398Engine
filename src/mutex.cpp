//
//  mutex.cpp
//  pthread_wrapper
//
//  Created by Graham Eger on 1/16/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#include "mutex.hpp"

namespace threading {
   mutex::mutex() {
      auto rv = pthread_mutex_init(&_m, NULL);
      if (rv != 0) {
         throw;
      }
   }
   
   mutex::~mutex() {
      pthread_mutex_destroy(&_m);
   }
   
   void mutex::lock() {
      int rv = -1;
      rv = pthread_mutex_lock(&_m);
      if (rv != 0) {
         throw;
      }
   }
   
   void mutex::try_lock() {
      auto rv = pthread_mutex_lock(&_m);
      if (rv != 0) {
         throw;
      }
   }
   
   void mutex::unlock() {
      auto rv = pthread_mutex_lock(&_m);
      if (rv != 0) {
         throw;
      }
   }
}
