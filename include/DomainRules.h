#ifndef DOMAINROBOTSTXTRULES_H
#define DOMAINROBOTSTXTRULES_H

class String;
class DirectoryRule;
struct Rule;

#include "String.h"
#include "TokenStream.h"
#include "RobotsTxt.h"
#include "DirectoryRules.h"

//TODO create desctructor
// TODO: Figure out how to deal with "*" wildcards in paths
//TODO: add domain member variable if necessary
class DomainRules {
public:
   DomainRules(const char* robotsFilename);
   //construct from existing DirectoryRules tree
   DomainRules(DirectoryRules* rootIn);
   bool IsAllowed(String path);
   void WriteRulesToDisc(std::string &domain);

private:
   void AddRule(Rule rule);

   DirectoryRules* root;
};

#endif