/*
 * expression.cpp
 *
 * Class implementations for expression functionality
 *
 */

#include "expression.h"

Expression::~Expression( )
   {
   }

Phrase::Phrase( string val ) : value(val) {}


string Phrase::stringEval( ) const
{
    return value;
}

void ANDExpression::addTerm(Expression * phrase) {
    terms.push_back(phrase);
}

void AddExpression::addTerm(Expression * phrase) {
    terms.push_back(phrase);
}

void OrExpression::addTerm(Expression * phrase) {
    terms.push_back(phrase);
}

