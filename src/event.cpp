//
//  event.cpp
//  engine
//
//  Created by Graham Eger on 2/2/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//
// 

#include "event.hpp"

namespace search {
    EventQueue::EventQueue() {
        t = std::thread(&EventQueue::process, this);
    }

    void EventQueue::process() {

    }


}

