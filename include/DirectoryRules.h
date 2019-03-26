#ifndef DIRECTORYRULES_H
#define DIRECTORYRULES_H

#include <string>
#include <set>
#include <vector>
#include <unordered_map>
<<<<<<< HEAD
#include <stdio.h>

using std::vector;
=======
>>>>>>> robots

//You must first create a root directory by calling DirectoryRules("/"). Create new children
//using FindOrCreateChild()
//TODO: create destructor
class DirectoryRules {
public:
<<<<<<< HEAD
   //TODO CREATE DESTRUCTOR
   DirectoryRules(std::string &name, bool allowed=true, bool hasRule=false);
   void SetAllowed(bool grantedPermission);
   //To jason: consider making SetAllowedForSubtree private? SetAllowed calls it and handles the logic.
   //Not sure if you ever need to call it outwardly
   void SetHasRule();
   bool GetAllowed();
   std::string GetDirectoryName();
   DirectoryRules *FindOrCreateChild(std::string &name);
   //Only use this if you are reading file and building tree from scratch
   void AddChildFromFile(DirectoryRules* child); 
   void SetChildIndices(vector<int> &childIndicesIn);
   //fp must point to empty file
   void SaveToFile(FILE *fp);
   vector<int> childIndicesInDstVec; //used for saving to file

private:
   void SetAllowedForSubtree(bool grantedPermission); 
   void GetVectorizedRules(vector<DirectoryRules*> &dstVec);
   void SaveRulesVector(FILE *fp);
=======
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
>>>>>>> robots
   std::vector<DirectoryRules*> childrenRules;
   std::string directoryName;
   bool isAllowed;
   bool hasRule;
   std::unordered_map<std::string, int> directoryNameToChildRuleIndex;
<<<<<<< HEAD
   void ReadRulesFromDisc(FILE *file, vector<DirectoryRules*> &rules);
   int FindEndIndexOfNextDirectoryName(std::string &path, int directoryStartIndex);
   void CreateDirectoryRuleTree(vector<DirectoryRules*> &rules);
};

#endif
=======

   int findEndIndexOfNextDirectoryName(std::string &path, int directoryStartIndex);
};

#endif
>>>>>>> robots
