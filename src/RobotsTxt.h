#ifndef ROBOTSTXT_H
#define ROBOTSTXT_H

#include <string>

class DirectoryRule;
struct Rule;

// TODO: Figure out how to deal with "*" wildcards in paths
class RobotsTxt {
public:
   RobotsTxt(std::string robotsFilename);
   bool IsAllowed(std::string path);

private:
   void AddRule(Rule rule);

   DirectoryRule* root;
};

#endif
