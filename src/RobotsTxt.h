#ifndef ROBOTSTXT_H
#define ROBOTSTXT_H

#include <string_view>

class DirectoryRule;
struct Rule;

// TODO: Figure out how to deal with "*" wildcards in paths
class RobotsTxt {
public:
   RobotsTxt(std::string_view robotsFilename);
   bool IsAllowed(std::string_view path);

private:
   AddRule(Rule rule);

   DirectoryRule root;
};

#endif
