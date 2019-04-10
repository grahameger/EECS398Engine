#pragma once
#ifndef DOMAINROBOTSTXTRULES_H
#define DOMAINROBOTSTXTRULES_H

class String;
class DirectoryRules;
struct Rule;

#include "String.h"
#include "TokenStream.h"
#include "RobotsTxt.h"
#include "DirectoryRules.h"

class DomainRules
   {
public:
   DomainRules( const char* robotsFilename );
   //construct from existing DirectoryRules tree
   DomainRules( DirectoryRules* rootIn );
   ~DomainRules();
   bool IsAllowed( String path );
   void WriteRulesToDisc( std::string &domain, std::string &serializedRulesFolder );

private:
   void AddRule( Rule rule );

   DirectoryRules* root;
   };

#endif