#ifndef ROBOTSTXT_H
#define ROBOTSTXT_H

class String;
class DirectoryRule;
struct Rule;

// TODO: Figure out how to deal with "*" wildcards in paths
class RobotsTxt {
public:
   RobotsTxt(const char* robotsFilename);
   bool IsAllowed(String path);

private:
   void AddRule(Rule rule);

   DirectoryRule* root;
   /*
   make a new class that acts as a container of RobotsTxt, where each RobotsTxt object within contains the 
   info for one domain's robot.txt rules
   
   The crawler class will contain an instance of this outter class, which will be passed to http.
   At the bottom of submitURL in http, populate the RobotsTxt for this domain using the given member functions

   This outter class will have both an lru cache of domains, as well as functionaltiy for searching disc
   whenever you miss the cache. All parsed robots Txt files will be saved to disc.

   Saving a robots.txt representation into disc will involve creating a tree->vector->file representation
   */
};

#endif
