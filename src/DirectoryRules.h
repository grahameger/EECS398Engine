#ifndef DIRECTORYRULES_H
#define DIRECTORYRULES_H

#include <string_view>
#include <set>

class DirectoryRules {
public:
   DirectoryRules(std::string_view name, bool allowed);
   void SetAllowedAsRoot(bool newAllowed);
   void SetAllowedForSubtree(bool newAllowed);
   void SetHasRule();

   DirectoryRules& FindOrCreateChild(std::string_view name);

private:
   std::set<DirectoryRules> childrenRules;
   std::string_view directoryName;
   bool isAllowed;
   bool hasRule;
};

#endif
