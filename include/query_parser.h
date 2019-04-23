// /*
//  * parser.h
//  *
//  * Basic query expression parser that supports phrase, and, or queries
//  *
#pragma once
#ifndef PARSER_H_
#define PARSER_H_

#include "expression.h"
#include "query_tokenstream.h"

/**
 * The actual expression parser
 */
class Parser
{
    // Stream of tokens to consume input from
    TokenStream stream;
    
    /**
     * Find the appropriate nonterminal
     *
     * Return nullptr if it could not be found
     */
    
    Expression *FindPhrase( );
    
    Expression *FindSimple( );
    
    Expression *FindAnd( );
    
    Expression *FindOr( );
    
    
public:
    
    /**
     * Construct parser based on given input
     */
    Parser( const std::string &in );
    
    bool fullParsed();
    
    /**
     * The public interface of the parser. Call this function,
     * rather than the private internal functions.
     */
    Expression *Parse( );
};
// class Parser

#endif /* PARSER_H_ */
