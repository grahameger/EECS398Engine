//
//  utf8.hpp
//  pthread_wrapper
//
//  Created by Graham Eger on 1/23/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#include "utf8.hpp"
#include <locale>
#include <codecvt>
#include <cassert>

namespace search {

    // should be doing math on the number of 
    // 7-bit words needed to store the integer c
    size_t SizeOfUtf8(Unicode c) {
        if (c < 0x007f) {
            return 1;
        } else if (c <= 0x07ff) {
            return 2;
        } else if (c <= 0xffff) {
            return 3;
        } else if (c <= 0x10ffff) {
            return 4; 
        }
    }
}