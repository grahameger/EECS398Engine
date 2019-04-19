/*
 * tokenstream.cpp
 *
 * Implementation of tokenstream.h
 *
 */

#include <assert.h>

#include <algorithm>

#include <iostream>

#include "query_tokenstream.h"


bool CharIsRelevant( char c )
{
    switch ( c )
    {
        case '@': //anchor text
        case '#': //title
        case '$': //url
        case '&':
        case '-':
        case '*': //body
        case '(':
        case ')':
        case '|':
        case '"':
        case ' ':
            return true;
        default:
            return isdigit( c ) || is_char( c );
    }
}

bool CharIsIrrelevant( char c )
{
    return !CharIsRelevant( c );
}

TokenStream::TokenStream( const std::string &in ) :
input( in )
{
    // Erase irrelevant chars using algorithm
    input.erase( std::remove_if( input.begin( ), input.end( ), CharIsIrrelevant ), input.end( ) );
}

bool TokenStream::Match( char c )
{
    if ( location >= input.size( ) )
    {
        return false;
    }
    while(input[ location ] == ' ') {
        ++location;//get rid of whitespace
    }
    if ( input[ location ] == c )
    {
        ++location;
        if ( c == '&' ) {
            match_and = true;
        }
        return true;
    }
    return false;
}

void TokenStream::Reset_location( )
{
    while(input[location] != '"')
    {
        location--;
    }
}

bool TokenStream::AllConsumed( ) const
{
    return location == input.size( );
}

Phrase *TokenStream::parseWord( )
{
    if ( location >= input.size( ) )
    {
        return nullptr;
    }
    string val = "";
    size_t start = location;
    while(location < input.size() && CharIsRelevant(input[location]) && input[location] != ' ' && input[location] != '"' && input[location] != '&' && input[location] != '|' && input[location] != ')' && input[location] != '(') {
        if(is_char(input[location])) {
            val += tolower(input[location]);
        }
        else {
            val += input[location];
        }
        location++;
    }
    
    if(location == start) { //no word at all
        return nullptr;
    }
    
    while(location < input.size() && input[location] == ' ') {
        location++;//get rid of whitespace
    }
    
    return new Phrase( val );
}

bool is_char(const char c) {
    return (((int)c >= 65 && (int)c <= 90) || ((int)c >= 97 && (int)c <= 122)) ? true : false;
}

bool TokenStream::last_char() {
    return location == input.length() - 1;
}

void help_message() {
    std::cout << "You entered an invalid query. Remember to close all tags. Valid characters are:\n";
    std::cout << "[A-Z], [a,z], [0-9], #, @, &, $, -, * |, (, ), \"\n\n";
    std::cout << "To AND two phrases, use &. To OR two phrases, use |\n\n";
    std::cout << "To exclude a word or phrase from your search prepend -\n";
    std::cout << "To search for words in title, prepend #\n";
    std::cout << "To search for words in url, prepend @\n";
    std::cout << "To search for words in anchor text, prepend $\n";
    std::cout << "To search for words in body text, prepend *\n";
}