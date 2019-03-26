#ifndef DIRECTORYRULES_H
#define DIRECTORYRULES_H

#include <string>
#include <set>
#include <vector>
#include <unordered_map>
#include <stdio.h>

using std::vector;

//You must first create a root directory by calling DirectoryRules("/"). Create new children
//using FindOrCreateChild()
//TODO: create destructor
class DirectoryRules
   {
public:
   //TODO CREATE DESTRUCTOR
   DirectoryRules( std::string name, bool allowed = true, bool hasRule = false );

   void SetAllowed( bool grantedPermission );
   void SetHasRule( );

   bool GetAllowed( );
   std::string GetDirectoryName( );

   DirectoryRules *FindOrCreateChild( std::string name );

   //Only use this if you are reading file and building tree from scratch
   void AddChildFromFile( DirectoryRules* child );
   void SetChildIndices( vector<int> &childIndicesIn );

   //fp must point to empty file
   void SaveToFile( FILE *fp );
   vector<int> childIndicesInDstVec; //used for saving to file

private:
   void SetAllowedForSubtree(bool grantedPermission); 
   void GetVectorizedRules(vector<DirectoryRules*> &dstVec);
   void SaveRulesVector(FILE *fp);
   std::vector<DirectoryRules*> childrenRules;
   std::string directoryName;
   bool isAllowed;
   bool hasRule;
   std::unordered_map<std::string, int> directoryNameToChildRuleIndex;
   void ReadRulesFromDisc(FILE *file, vector<DirectoryRules*> &rules);
   int FindEndIndexOfNextDirectoryName(std::string &path, int directoryStartIndex);
   void CreateDirectoryRuleTree(vector<DirectoryRules*> &rules);
   };

#endif
