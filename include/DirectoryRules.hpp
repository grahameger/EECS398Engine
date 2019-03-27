#pragma once
#ifndef DIRECTORYRULES_H
#define DIRECTORYRULES_H

#include <string>
#include <set>
#include <vector>
#include <unordered_map>

//You must first create a root directory by calling DirectoryRules("/"). Create new children
//using FindOrCreateChild()
//TODO: create destructor
class DirectoryRules {
public:
   DirectoryRules(std::string &name, bool allowed=true);
   void SetAllowed(bool grantedPermission);
   //To jason: consider making SetAllowedForSubtree private? SetAllowed calls it and handles the logic.
   //Not sure if you ever need to call it outwardly
   void SetAllowedForSubtree(bool grantedPermission); 
   void SetHasRule();
   bool GetAllowed();
   std::string GetDirectoryName();

   DirectoryRules *FindOrCreateChild(std::string &name);

private:
   std::vector<DirectoryRules*> childrenRules;
   std::string directoryName;
   bool isAllowed;
   bool hasRule;
   std::unordered_map<std::string, int> directoryNameToChildRuleIndex;

   int findEndIndexOfNextDirectoryName(std::string &path, int directoryStartIndex);
};

#endif
