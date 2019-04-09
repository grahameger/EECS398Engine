#pragma once

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
class DirectoryRules
   {
public:
   DirectoryRules( std::string name, bool allowed = true, bool hasRule = false );
   ~DirectoryRules( );

   void SetAllowed( bool grantedPermission );
   void SetHasRule( );

   bool GetAllowed( );
   std::string GetDirectoryName( );

   DirectoryRules *FindOrCreateChild( std::string name );

   //Only use this if you are reading file and building tree from scratch
   void AddChildFromFile( DirectoryRules* child );
   void SetChildIndices( vector<size_t> &childIndicesIn );

   //fp must point to empty file
   void SaveToFile( FILE *fp );
   vector<size_t> childIndicesInDstVec; //used for saving to file

   static const size_t npos = -1;

private:
   void SetAllowedForSubtree(bool grantedPermission); 
   void GetVectorizedRules(vector<DirectoryRules*> &dstVec);
   void SaveRulesVector(FILE *fp);
   std::vector<DirectoryRules*> childrenRules;
   std::string directoryName;
   bool isAllowed;
   bool hasRule;
   std::unordered_map<std::string, size_t> directoryNameToChildRuleIndex;
   void ReadRulesFromDisc(FILE *file, vector<DirectoryRules*> &rules);
   size_t FindEndIndexOfNextDirectoryName(std::string &path, size_t directoryStartIndex);
   void CreateDirectoryRuleTree(vector<DirectoryRules*> &rules);
   };

#endif
