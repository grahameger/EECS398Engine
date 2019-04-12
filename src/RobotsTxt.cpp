// Created by Dennis Li
// Graham Eger added threading functions on 3/29

#include "RobotsTxt.h"

const size_t CACHE_CAPACITY = 100000;

RobotsTxt::RobotsTxt()
   : domainRulesCache(CACHE_CAPACITY, true), SerializedRulesPath("SerializedRobotsRules")
   {
   makeDir(SerializedRulesPath.c_str());
   }

RobotsTxt::~RobotsTxt()
   {
   domainRulesCache.clear();
   }

void RobotsTxt::SubmitRobotsTxt(string &domain, string &pathOnDisc)
   {
   DomainRules *newDomainRules = new DomainRules(pathOnDisc.c_str());

   domainRulesCache.put(domain, newDomainRules);
   newDomainRules->WriteRulesToDisc(domain, SerializedRulesPath);
   }

DirectoryRules *RobotsTxt::CreateDirectoryRules(char *directoryName, 
   vector<size_t> &childIndices, bool isAllowed, bool hasRule)
   {
   string dirNameStr(directoryName);
   DirectoryRules *newDirRule = new DirectoryRules(dirNameStr, isAllowed, hasRule);
   newDirRule->SetChildIndices(childIndices);
   return newDirRule;
   }

void RobotsTxt::ReadRulesFromDisc(FILE *file, vector<DirectoryRules*> &rules)
   {
   char curChar = fgetc(file);
   while(curChar != EOF)
       {
       //get name of directory
       char dirName[99];
       size_t dirNameInd = 0;

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
           hasRule = true;
           }

       //children indices
       std::vector<size_t> childrenInd;
       curChar = (char)fgetc(file);
       while(curChar != '\n' && curChar != EOF && curChar != ' ')
          {
          size_t index = curChar - '0';
          curChar = (char)fgetc(file); //process the space
          //handle multi digit numbers
          while(curChar != ' ' && curChar != EOF)
             {
             index *= 10;
             index += curChar - '0';
             curChar = (char)fgetc(file); //process the space
             }

         childrenInd.push_back(index);
         //get next index char
         curChar = (char)fgetc(file);
         }
      
      if(curChar == ' ') curChar = (char)fgetc(file);

       //create new DirectoryRules object
       DirectoryRules *newRule = CreateDirectoryRules(dirName, 
          childrenInd, isAllowed, hasRule); 
       rules.push_back(newRule);
       
       curChar = (char)fgetc(file);
       }
   }

void RobotsTxt::CreateDirectoryRuleTree(vector<DirectoryRules*> &rules)
   {
   for(size_t i = 0; i < rules.size(); ++i)
      {
      vector<size_t> &indsInRulesVec = rules[i]->childIndicesInDstVec;
      for(size_t j = 0; j < indsInRulesVec.size(); ++j)
         {
         size_t childInd = indsInRulesVec[j];
         DirectoryRules *childRule = rules[childInd];
         rules[i]->AddChildFromFile(childRule);
         }
       }
   }

//returns false if file does not exist
bool RobotsTxt::TransferRulesFromDiscToCache(string &domain)
   {
   string fileName = domain;
   //string fileName = "dennisli.txt";
   FILE *file = fopen(fileName.c_str(), "r");
   //file doesn't exist
   if(!file)
      {
      printf("File doesn't exist!");
      return false;
      }

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
   return true;
   } 

bool RobotsTxt::GetRule(string path, string &domain)
   { 
   //strip ending '/' from directory names to keep consistent
   if(path.back() == '/' && path.size() > 1)
      path.pop_back();
   
   //Look in cache
   String tmpPath(path.c_str());
   try
      {
      DomainRules *domainRule = domainRulesCache.get(domain);
      return domainRule->IsAllowed(tmpPath);
      }
   catch( ... )
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

void RobotsTxt::lock() {
   m.lock();
}

void RobotsTxt::unlock() {
   m.unlock();
}

void RobotsTxt::makeDir(const char * name) {
   struct stat st = {0};
   if (stat(name, &st) == -1) {
      int rv = mkdir(name, 0755);
      if (rv == -1) {
         fprintf(stderr, "error creating directory %s - %s\n", name, strerror(errno));
         exit(1);
      } else {
         fprintf(stdout, "created directory %s\n", name);
      }
   }
}
