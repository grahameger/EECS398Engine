/*
 * tokenstream.h
 *
 * Declaration of a stream of tokens that you can read from
 *
 */
#pragma once
#ifndef TOKENSTREAM_H_
#define TOKENSTREAM_H_

#include <string>
#include "expression.h"
#include <cstring>


/*
 * Check if a character is relevant to a math expression
 *
 * Relevant characters are defined as
 * +
 * -
 * *
 * (
 * )
 *
 * and all digits
 */
bool CharIsRelevant( char c );

/**
 * Opposite of char is relevant. Needed for filtering the input
 */
bool CharIsIrrelevant( char c );

/**
 * The token stream, which you can both Match( ) a single character from,
 * or ParseWord( ) to consume a whole word
 *
 * The input string by default is filtered of any characters that are
 * deemed "irrelevant" by the CharIsIrrelevant function above
 */

bool is_char( const char c );

void help_message( );


class TokenStream
{
    // The input we receive, with only relevant characters left
    std::string input;
    // Where we currently are in the input
    size_t location { 0 };
    
public:
    
    void Reset_location( );
    
    /**
     * Construct a token stream that uses a copy of the input
     * that contains only characters relevant to math expressions
     */
    TokenStream( const std::string &in );
    
    /**
     * Attempt to match and consume a specific character
     *
     * Returns true if the char was matched and consumed, false otherwise
     */
    bool Match( char c );
    bool match_and = false;
    bool last_char( );
    
    
    /**
     * Check whether all the input was consumed
     */
    bool AllConsumed( ) const;
    
    
    /**
     * Attempt to match and consume a whole word
     *
     * Return a dynamically allocated phrase if successful, nullptr otherwise
     */
    
    Phrase *parseWord( );
    
};

#endif /* TOKENSTREAM_H_ */
