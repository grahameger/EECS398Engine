/*
 * parser.cpp
 *
 * Implementation of parser.h
 *
 */

#include "expression.h"
#include "query_parser.h"

/* <OrConstraint>  ::=   <AndConstraint> { <OrOp> <AndConstraint> }
 *
 * <AndConstraint>  ::= <SimpleConstraint> { [ <AndOp> ] <SimpleConstraint> }
 *
 * <SimpleConstraint>  ::= <Phrase> | ‘(’ <OrConstraint> ‘)’| <UnaryOp> <SimpleConstraint> | <SearchWord>
 *
 * <Phrase>  ::= '"' { <SearchWord> } '"'
 */

Expression *Parser::FindPhrase( ) // "bill (nye|gates)" turn into??
{
    Expression *word = stream.parseWord();
    if( word ) {
        AddExpression *self = new AddExpression( );
        self->addTerm( word );
        while( ( word = stream.parseWord() )) {
            self->addTerm( word );
        }
        return self;
    }
    
    return nullptr;
    
}

Expression *Parser::FindOr() {
    Expression *left = FindAnd( );
    if ( left )
    {
        OrExpression *self = new OrExpression( );
        self->addTerm( left );
        while ( stream.Match( '|' ) )
        {
            left = FindOr( );
            if( !left ) {
                return nullptr;
            }
            self->addTerm( left );
            // ...
        }
        return self;
    }
    return nullptr;
}

Expression *Parser::FindAnd() {
    Expression *left = FindSimple( );
    if ( left )
    {
        ANDExpression *self = new ANDExpression( );
        self->addTerm( left );
        while ( (left = FindSimple( )) || stream.Match( '&' ))
        {
            if(stream.match_and) {
                left = FindSimple( );
                stream.match_and = false;
            }
            if( !left ) {
                return nullptr;
            }
            self->addTerm( left );
            if(stream.last_char()) {
                return self;
            }
            // ...
        }
        return self;
    }
    return nullptr;
}


Expression *Parser::FindSimple() {
    
    if ( stream.Match( '"' ) ) {// PHRASE
        Expression *left = FindPhrase( );
        if ( left )
        {
            AddExpression *self = new AddExpression( );
            self->addTerm( left );
            if(!stream.Match('"')) {
                return nullptr;//must be closing
            }
            return self;
        }
    }
    else if(stream.Match( '(' ) ) {//OR CONSTRAINT
        Expression *left = FindOr( );
        if ( left )
        {
            ParenthOrExpression *self = new ParenthOrExpression( );
            self->addTerm( left );
            if(!stream.Match(')')) {
                return nullptr;//must be closing
            }
            return self;
        }
    }
    else if(stream.Match('+') || stream.Match('-')) {//UNARY + SIMPLE
        Expression *left = FindSimple( );
        if ( left )
        {
            SubExpression *self = new SubExpression( );
            self->addTerm( left );
            return self;
        }
    }
    
    else {
        return stream.parseWord();
    }
    
    
    
    return nullptr;
}

Expression *Parser::Parse( )
{
    return FindOr();
    
}

bool Parser::fullParsed() {
    return stream.AllConsumed();
}

Parser::Parser( const std::string &in ) :
stream( in )
{
}