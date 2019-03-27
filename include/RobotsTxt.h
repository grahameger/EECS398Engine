#pragma once

#ifndef ROBOTSTXT_H_398
#define ROBOTSTXT_H_398

#include <string>
#include "http.hpp"
#include "String.h"
#include "LRUCache.hpp"
#include "DomainRules.h"
#include "DirectoryRules.h"


namespace search {
   struct HTTPRequest;
}

using std::string;
using std::vector;
using search::HTTPRequest;

//TODO move these params to central yaml file
extern const size_t CACHE_CAPACITY;

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
   void SubmitRobotsTxt(string &domain, string &pathOnDisc); 
   //return true if allowed to crawl. false otherwise
   //path is the path of the http file you are getting the rule for, e.g. "/personal/dennis/resume.http"
   //TODO: the arguments of this function should be grouped into class
   //that should be constructed upon getting a new file (http must be changed)
   bool GetRule(string &path, string &domain);

private:
   LRUCache<string, DomainRules*> domainRulesCache;
   void ReadRulesFromDisc(FILE *file, vector<DirectoryRules*> &rules);
   bool TransferRulesFromDiscToCache(string &domain);
   DirectoryRules *CreateDirectoryRules(char *directoryName, 
      vector<int> &childIndices, bool isAllowed, bool hasRule);
   void CreateDirectoryRuleTree(vector<DirectoryRules*> &rules);
};

#endif
