#include "DirectoryRules.h"
#include <iostream>
using std::set;
using std::string;

DirectoryRules::DirectoryRules(string name, bool allowed, bool hasRuleIn) 
    : directoryName(name), isAllowed(allowed), hasRule(hasRuleIn) {}

//Return index one past the last letter of the next directory name
int DirectoryRules::FindEndIndexOfNextDirectoryName(string &path, int directoryStartIndex)
{
    int tmpIndex = directoryStartIndex;
    while(path[tmpIndex] != '/' && tmpIndex < path.size())
    {
        tmpIndex++;
    }
    return tmpIndex;
}

void DirectoryRules::SetChildIndices(vector<int> &childIndicesIn)
{
   childIndicesInDstVec = childIndicesIn;
}

DirectoryRules* DirectoryRules::FindOrCreateChild(string path)
{
    //edge case: user searches for permissions of directory "/"
    if(path == "/")
    {
        if(directoryName == path) return this;
        else 
        {
            //TODO: throw exception
            //user either is attemting to create a subdirectory named "/" or
            //the root folder is not named "/"
            std::cerr << "Invalid path creation!";
            exit(1);
        }
    }

    int currentDirectoryStartIndex = 1; //start at 1 to ignore '/'
    DirectoryRules *currentDirectoryRule = this;

    while(currentDirectoryStartIndex <= path.size())
    {
        int currentDirectoryEndIndex = FindEndIndexOfNextDirectoryName(path, currentDirectoryStartIndex);
        string currentDirectoryName = path.substr(currentDirectoryStartIndex, 
            currentDirectoryEndIndex - currentDirectoryStartIndex);

        int currentDirectoryRuleIndex = -1;

        if(currentDirectoryRule->directoryNameToChildRuleIndex.find(currentDirectoryName) == 
            currentDirectoryRule->directoryNameToChildRuleIndex.end())
        {
            DirectoryRules *newDirectoryRule = new DirectoryRules(currentDirectoryName, isAllowed);
            currentDirectoryRule->childrenRules.push_back(newDirectoryRule);
            currentDirectoryRuleIndex = currentDirectoryRule->childrenRules.size() - 1;
            currentDirectoryRule->directoryNameToChildRuleIndex.insert({currentDirectoryName, currentDirectoryRuleIndex});
        }

        else currentDirectoryRuleIndex = currentDirectoryRule->directoryNameToChildRuleIndex[currentDirectoryName];

        currentDirectoryRule = currentDirectoryRule->childrenRules[currentDirectoryRuleIndex];
        currentDirectoryStartIndex = currentDirectoryEndIndex + 1;
    }

    return currentDirectoryRule;
}


void DirectoryRules::SetAllowedForSubtree(bool grantedPermission)
{
    if(hasRule) return;
    isAllowed = grantedPermission;
    for(int i = 0; i < childrenRules.size(); ++i)
    {
        childrenRules[i]->SetAllowedForSubtree(grantedPermission);
    }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
}

void DirectoryRules::SetAllowed(bool grantedPermission)
{
    isAllowed = grantedPermission;
    SetHasRule();

    for(int i = 0; i < childrenRules.size(); ++i)
    {
        childrenRules[i]->SetAllowedForSubtree(grantedPermission);
    }
}

bool DirectoryRules::GetAllowed()
{
    return isAllowed;
}

string DirectoryRules::GetDirectoryName()
{
    return directoryName;
}

void DirectoryRules::SetHasRule()
{
    hasRule = true;
}

void DirectoryRules::GetVectorizedRules(vector<DirectoryRules*> &dstVec)
{
    for(int i = 0; i < childrenRules.size(); ++i)
    {
        dstVec.push_back(childrenRules[i]);
        
        int indInDstVec = dstVec.size() - 1;
        childIndicesInDstVec.push_back(indInDstVec);

        childrenRules[i]->GetVectorizedRules(dstVec);
    }
}

void DirectoryRules::SaveRulesVector(FILE *fp)
{
    fprintf(fp, directoryName.c_str());
    fprintf(fp, " ");

    if(isAllowed)
        fprintf(fp, "1");
    else
        fprintf(fp, "0");
    //note no space

    if(hasRule)
        fprintf(fp, "1");
    else
        fprintf(fp, "0");
    //note no space
    
    for(int i = 0; i < childIndicesInDstVec.size(); ++i)
    {
        fprintf(fp, "%d ", childIndicesInDstVec[i]);
    }

    fprintf(fp, "\n");
}

void DirectoryRules::SaveToFile(FILE *fp)
{
    vector<DirectoryRules*> rulesVec;
    rulesVec.push_back(this);
    GetVectorizedRules(rulesVec);

    for(int i = 0; i < rulesVec.size(); ++i)
    {
        rulesVec[i]->SaveRulesVector(fp);
    }
}

void DirectoryRules::AddChildFromFile(DirectoryRules* child)
{
    childrenRules.push_back(child);
    directoryNameToChildRuleIndex.insert({child->GetDirectoryName(), childrenRules.size() - 1});
}
