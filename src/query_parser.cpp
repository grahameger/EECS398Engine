// /*
//  * parser.cpp
//  *
//  * Implementation of parser.h
//  *
//  */

#include "expression.h"
#include "query_parser.h"

ISR *Parser::FindPhrase( )
 {
 ISR *word = stream.parseWord( );
 if( word ) {
 ISRPhrase *self = new ISRPhrase( );
 self->addTerm( word );
 while( ( word = stream.parseWord( ) ) )
 {
 self->addTerm( word );
 }
 return self;
 }
 
 return nullptr;
 
 }

ISR *Parser::FindOr( )
{
   ISR *left = FindAnd( );
   if ( left )
   {
      ISROr *self = new ISROr( );
      self->addTerm( left );
      while ( stream.Match( '|' ) )
      {
         left = FindOr( );
         if( !left )
         {
            return nullptr;
         }
         self->addTerm( left );
         // ...
      }
      return self;
   }
   return nullptr;
}

ISR *Parser::FindAnd( )
{
   ISR *left = FindSimple( );
   if ( left )
   {
      ISRAnd *self = new ISRAnd( );
      self->addTerm( left );
      while ( ( left = FindSimple( ) ) || stream.Match( '&' ) )
      {
         if( stream.match_and )
         {
            left = FindSimple( );
            stream.match_and = false;
         }
         if( !left ) {
            return nullptr;
         }
         self->addTerm( left );
         if( stream.last_char( ) )
         {
            return self;
         }
      }
      return self;
   }
   return nullptr;
}


ISR *Parser::FindSimple( )
{
   
   if ( stream.Match( '"' ) ) 
   {
    ISR *left = FindPhrase( );
    if ( left )
    {
    ISRPhrase *self = new ISRPhrase( );
    self->addTerm( left );
    if( !stream.Match( '"' ) )
    {
    return nullptr;
    }
    return self;
    }
    }
    else */if(stream.Match( '(' ) )
    {
       ISR *left = FindOr( );
       if ( left )
       {
          ISROr *self = new ISROr( );
          self->addTerm( left );
          if( !stream.Match( ')' ) )
          {
             return nullptr;
          }
          return self;
       }
    }
      else {
       return stream.parseWord( );
    } 
   return nullptr;
}

ISR *Parser::Parse( )
{
   return FindOr( );
   
}

bool Parser::fullParsed( )
{
   return stream.AllConsumed();
}

Parser::Parser( const std::string &in ) :
stream( in )
{
}
