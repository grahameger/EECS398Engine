/*
 * expression.h
 *
 * Class declarations for expressions
 *
 */

#pragma once
#ifndef EXPRESSION_H_
#define EXPRESSION_H_

#include <stdint.h>
#include <vector>
#include <cstddef>
#include <string>
using namespace std;

/**
 * Just a plain old expression
 */
class Expression
{
public:
    
    virtual ~Expression( );
    
    //virtual int64_t Eval( ) const = 0;
    virtual string stringEval( ) const = 0;
};
// class Expression
class Phrase: public Expression
{
protected:
    
    string value;
    
public:
    
    Phrase( string val );
    
    string stringEval( ) const override;
};

class ANDExpression : public Expression
{
protected:
    std::vector < Expression * > terms;
public:
    void addTerm(Expression *);
    
    string stringEval( ) const override
    {
        string phrase = terms[ 0 ]->stringEval( );
        for ( size_t i = 1;  i < terms.size( );  ++i ) {
            phrase += "&" + terms[ i ]->stringEval( );
        }
        return phrase;
    }
};

class AddExpression : public Expression
{
protected:
    std::vector < Expression * > terms;
public:
    void addTerm(Expression *);
    
    string stringEval( ) const override
    {
        string phrase = terms[ 0 ]->stringEval( );
        for ( size_t i = 1;  i < terms.size( );  ++i ) {
            phrase += "&" + terms[ i ]->stringEval( );
        }
        return phrase;
    }
};

class OrExpression : public Expression
{
protected:
    std::vector < Expression * > terms;
public:
    void addTerm(Expression *);
    
    string stringEval( ) const override
    {
        string phrase = terms[ 0 ]->stringEval( );
        for ( size_t i = 1;  i < terms.size( );  ++i ) {
            phrase += "|" + terms[ i ]->stringEval( );
        }
        return phrase;
    }
};

#endif /* EXPRESSION_H_ */
