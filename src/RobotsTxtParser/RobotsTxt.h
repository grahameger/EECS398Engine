#ifndef ROBOTSTXT_H
#define ROBOTSTXT_H

#include <string_view>
#include "DirectoryRules.h"

using std::string_view;

// TODO: Figure out how to deal with "*" wildcards in paths
class RobotsTxt {
public:
   RobotsTxt(string_view robotsFilename);
   bool IsAllowed(string_view path);

private:
   void Allow(string_view path);
   void Disallow(string_view path);

   DirectoryRule root;
};

#endif
