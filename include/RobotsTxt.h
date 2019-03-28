#ifndef ROBOTSTXT_H
#define ROBOTSTXT_H
#include <string>
#include "LRUCache.hpp"
#include "DomainRules.h"
#include "DirectoryRules.h"
#include "http.hpp"

using std::string;
using std::vector;
using search::HTTPRequest;

//TODO move these params to central yaml file
extern const size_t CACHE_CAPACITY;

//Our crawler will have one instance of this RobotsTxt class. 
//todo: change to our String and Hashtable class. Can't do until both are complete
//For now we assume the filename of the robots.txt file will be the domain
//Only one instance of RobotsTxt class should ever exist! It will be a wrapper for
//robots rules of all domains
class RobotsTxt {
public:
   RobotsTxt();
   ~RobotsTxt();
   //Calling this function will replace any existing robotsTxt info
   //for this domain. It will always trigger both a store-in-cache
   //and save-to-disc
   void SubmitRobotsTxt(string &domain, string &pathOnDisc); 
   //return true if allowed to crawl. false otherwise
   //path is the path of the http file you are getting the rule for, e.g. "/personal/dennis/resume.http"
   //As of now, we will save and search for all robotstxt parsed rules files in the current directory
   //GRAHAM - READ : You can change path to pass by reference if you are okay with path
   //getting changed on an edge case. As is, we copy the path string every time :(
   //If you do this please leave a warning
   bool GetRule(string path, string &domain);

private:
   LRUCache<string, DomainRules*> domainRulesCache;
   void ReadRulesFromDisc(FILE *file, vector<DirectoryRules*> &rules);
   bool TransferRulesFromDiscToCache(string &domain);
   DirectoryRules *CreateDirectoryRules(char *directoryName, 
      vector<int> &childIndices, bool isAllowed, bool hasRule);
   void CreateDirectoryRuleTree(vector<DirectoryRules*> &rules);
};

#endif
