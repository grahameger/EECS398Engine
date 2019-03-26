//todo change these to adhere to makefile
#include "http.hpp"
#include "RobotsTxt.h"

RobotsTxt::RobotsTxt()
   : domainRulesCache(CACHE_CAPACITY) {}

void RobotsTxt::SubmitRobotsTxt(HTTPRequest &robotsTxtHTTPInfo, string &pathOnDisc) //given url?
   {
   string &domain = robotsTxtHTTPInfo.host;

   DomainRules *newDomainRules = new DomainRules(pathOnDisc.c_str());//todo implement and ask if this is what DomainRules expects
   domainRulesCache.put(domain, newDomainRules);
   
   newDomainRules->WriteRulesToDisc();
   }

DirectoryRules *RobotsTxt::CreateDirectoryRules(char *directoryName, 
   vector<int> &childIndices, bool isAllowed, bool hasRule)
   {
   string dirNameStr(directoryName);
   DirectoryRules *newDirRule = new DirectoryRules(dirNameStr, isAllowed, hasRule);
   newDirRule->SetChildIndices(childIndices);
   return newDirRule;
   }

void RobotsTxt::ReadRulesFromDisc(FILE *file, vector<DirectoryRules*> &rules)
   {
   char curChar = (char)fgetc(file);
   while(curChar != EOF)
       {
       //get name of directory
       char dirName[99];
       int dirNameInd = 0;

       while(curChar != ' ')
       {
       dirName[dirNameInd++] = curChar;
       curChar = (char)fgetc(file);
       }

       dirName[dirNameInd] = 0;
       string dirNameStr(dirName);

       //isAllowed
       bool isAllowed = false;
       curChar = (char)fgetc(file);

       if(curChar == '1')
           {
           isAllowed = true;
           }

       //hasRule
       bool hasRule = false;
       curChar = (char)fgetc(file);

       if(curChar == '1')
           {
           isAllowed = true;
           }

       //children indices
       std::vector<int> childrenInd;
       while(curChar != '\n' && curChar != EOF)
           {
           childrenInd.push_back(curChar - '0');
           curChar = (char)fgetc(file);
           }

       //create new DirectoryRules object
       DirectoryRules *newRule = CreateDirectoryRules(dirName, 
          childrenInd, isAllowed, hasRule); 
       rules.push_back(newRule);

       //read in newLine
       curChar = (char)fgetc(file);
       }
   }

void RobotsTxt::CreateDirectoryRuleTree(vector<DirectoryRules*> &rules)
   {
   for(int i = 0; i < rules.size(); ++i)
      {
      vector<int> &indsInRulesVec = rules[i]->childIndicesInDstVec;
      for(int j = 0; j < indsInRulesVec.size(); ++j)
         {
         int childInd = indsInRulesVec[j];
         DirectoryRules *childRule = rules[childInd];
         rules[i]->AddChildFromFile(childRule);
         }
       }
   }

//returns false if file does not exist
bool RobotsTxt::TransferRulesFromDiscToCache(string &domain)
   {
   string fileName = domain + ".txt";
   FILE *file = fopen(fileName.c_str(), "r");
   //file doesn't exist
   if(!file)
      return false;

   vector<DirectoryRules*> rules;
   ReadRulesFromDisc(file, rules);

   fclose(file);

   //convert to DomainRules object and place in cache
   CreateDirectoryRuleTree(rules);
   if(rules.empty())
      {
      printf("No root folder!");
      return false;
      }

   DirectoryRules *root = rules[0];
   DomainRules *newRule = new DomainRules(root);
   domainRulesCache.put(domain, newRule);
   } 

bool RobotsTxt::GetRule(string &path, string &domain)
   { 
   //Look in cache
   String tmpPath(path.c_str()); //todo remove
   try
      {
      DomainRules *domainRule = domainRulesCache.get(domain);
      return domainRule->IsAllowed(tmpPath);
      }
   //search disc
   catch(const std::exception& e)
      {
      bool transferSuccess = TransferRulesFromDiscToCache(domain);
      if(!transferSuccess)
         {
         printf("This domain is not on file!");
         throw(1);
         }

      DomainRules *domainRule = domainRulesCache.get(domain);
      return domainRule->IsAllowed(tmpPath);
      }
   }