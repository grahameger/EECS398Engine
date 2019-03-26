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
class DomainRules {
public:
   DomainRules(const char* robotsFilename);
   //construct from existing DirectoryRules tree
   DomainRules(DirectoryRules* root);
   bool IsAllowed(String path);
   void WriteRulesToDisc();

private:
   void AddRule(Rule rule);

   DirectoryRule* root;
};

#endif