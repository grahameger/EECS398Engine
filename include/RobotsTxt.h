#ifndef ROBOTSTXT_H
#define ROBOTSTXT_H
<<<<<<< HEAD
#include <string>
#include "LRUCache.h"
#include "DomainRules.h"

using std::string;
using std::vector;

size_t CACHE_CAPACITY = 3;

//Our crawler will have one instance of this RobotsTxt class. 
//todo: change to our String and Hashtable class. Can't do until both are complete
//for now we assume the filename of the robots.txt file will be the domain

//Only one instance of RobotsTxt class should ever exist! It will be a wrapper for
//robots rules of all domains
class RobotsTxt {
public:
   RobotsTxt();
   //Calling this function will replace any existing robotsTxt info
   //for this domain. It will always trigger both a store-in-cache
   //and save-to-disc
   void submitRobotsTxt(HTTPRequest &robotsTxtHTTPInfo, string &pathOnDisc); 
   //return true if allowed to crawl. false otherwise
   //path is the path of the http file you are getting the rule for, e.g. "/personal/dennis/resume.http"
   //TODO: the arguments of this function should be grouped into class
   //that should be constructed upon getting a new file (http must be changed)
   bool getRule(string &path, string &domain);

private:
   LRUCache<string, DomainRules*> domainRulesCache;
   void ReadRulesFromDisc(FILE *file, vector<DirectoryRules*> &rules)
   bool TransferRulesFromDiscToCache(string &domain)
   DirectoryRules *RobotsTxt::CreateDirectoryRules(char *directoryName, 
      vector<int> &childIndices, bool isAllowed, bool hasRule)
   void CreateDirectoryRuleTree(vector<DirectoryRule*> &rules);
};

#endif
=======

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

   Consider making it a templated virtual function, as the functionality of check lru cache otherwise get file
   will be common
   */
};

#endif
>>>>>>> robots
