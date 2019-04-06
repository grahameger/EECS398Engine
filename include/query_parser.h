// /*
//  * parser.h
//  *
//  * Basic query expression parser that supports phrase, and, or queries
//  *
//  * A basic BNF query language:
//  *
//  * <OrConstraint>  ::=   <AndConstraint> { <OrOp> <AndConstraint> }
//  *
//  * <AndConstraint>  ::= <SimpleConstraint> { [ <AndOp> ] <SimpleConstraint> }
//  *
//  * <SimpleConstraint>  ::= <Phrase> | ‘(’ <OrConstraint> ‘)’| <UnaryOp> <SimpleConstraint> | <SearchWord>
//  *
//  * <Phrase>  ::= '"' { <SearchWord> } '"'
//  *
//  */

// #pragma once
// #ifndef QUERY_PARSER_H_
// #define QUERY_PARSER_H_

// #include <string>

// #include "expression.h"
// #include "TokenStream.h"

// /**
//  * The actual expression parser
//  */
// class Parser
//    {
//    // Stream of tokens to consume input from
//    TokenStream stream;

//    /**
//     * Find the appropriate nonterminal
//     *
//     * Return nullptr if it could not be found
//     */
       
//        Expression *FindPhrase( );
       
//        Expression *FindSimple( );
       
//        Expression *FindAnd( );
       
//        Expression *FindOr( );
       

// public:

//    /**
//     * Construct parser based on given input
//     */
//    Parser( const std::string &in );
       
//    bool fullParsed();

//    /**
//     * The public interface of the parser. Call this function,
//     * rather than the private internal functions.
//     */
//    Expression *Parse( );
//    };
// // class Parser

// #endif /* QUERY_PARSER_H_ */
