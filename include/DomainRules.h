#ifndef DOMAINROBOTSTXTRULES_H
#define DOMAINROBOTSTXTRULES_H

class String;
class DirectoryRules;
struct Rule;

#include "String.h"
#include "TokenStream.h"
#include "RobotsTxt.h"
#include "DirectoryRules.h"

//TODO create desctructor
// TODO: Figure out how to deal with "*" wildcards in paths
class DomainRules
   {
public:
   DomainRules( const char* robotsFilename );
   //construct from existing DirectoryRules tree
   DomainRules( DirectoryRules* rootIn );
   bool IsAllowed( String path );
   void WriteRulesToDisc( std::string &domain );

private:
   void AddRule( Rule rule );

   DirectoryRules* root;
   };

#endif
/*
make a new class that acts as a container of RobotsTxt, where each RobotsTxt object within contains the 
info for one domain's robot.txt rules

The crawler class will contain an instance of this outter class, which will be passed to http.
At the bottom of submitURL in http, populate the RobotsTxt for this domain using the given member functions

This outter class will have both an lru cache of domains, as well as functionaltiy for searching disc
whenever you miss the cache. All parsed robots Txt files will be saved to disc.

Saving a robots.txt representation into disc will involve creating a tree->vector->file representation

Consider making it a templated virtual function, as the functionality of check lru cache otherwise get file
will be common
*/
