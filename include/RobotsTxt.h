#ifndef ROBOTSTXT_H
#define ROBOTSTXT_H

#include "DirectoryRules.h"

class String;
struct Rule;

// TODO: Figure out how to deal with "*" wildcards in paths
class RobotsTxt {
public:
   RobotsTxt(const char* robotsFilename);
   bool IsAllowed(String path);

private:
   void AddRule(Rule rule);

   DirectoryRules* root;
};

#endif
