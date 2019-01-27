//
//  utf8.hpp
//  pthread_wrapper
//
//  Created by Graham Eger on 1/23/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#ifndef utf8_hpp_398
#define utf8_hpp_398

#include <stdio.h>
#include <iostream>
#include <string>

namespace search {

    using Unicode = uint32_t;
    using Utf8 = char;

    // SizeOfUtf8 tells the number of bytes it will take to encode the
    // specified Unicode value.
    size_t SizeOfUtf8(Unicode c);
    
    // Get the UTF-8 character as a Unicode value.
    // If it's an invalid UTF-8 encoding for a U-16
    // character, return the special malformed
    // character code.
    Unicode GetUtf8( Utf8 * p);
    
    // NextUtf8 will scan forward to the next byte
    // which could be the start of a UTF-8 character.
    // If it's on a null character, it scans over it.
    Utf8 * NextUtf8(Utf8 * p);

    // Scan backward for the first PREVIOUS byte which could
    // be the start of a UTF-8 character.
    Utf8 * PreviousUtf8(Utf8 * p);

    // Write a Unicode character in UTF-8.    
    Utf8 * WriteUtf8( Utf8 * p, Unicode c);

    // UTF-8 String compares.
    // Same return values as strcmp( ).
    int StringCompare(Utf8 * a, Utf8 * b);

    // Unicode string compare up to 'N' UTF-8 characters (not bytes)
    // from two UTF-8 strings.
    int StringCompare( Utf8 * a, Utf8 * b, size_t n);
} 


#endif /* utf8_hpp */