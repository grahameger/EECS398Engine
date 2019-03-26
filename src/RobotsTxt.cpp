<<<<<<< HEAD
//todo change these to adhere to makefile
#include "http.hpp"
#include "RobotsTxt.h"

void RobotsTxt::submitRobotsTxt(HTTPRequest &robotsTxtHTTPInfo, string &pathOnDisc) //given url?
   {
   string &domain = robotsTxtHTTPInfo.host;

   DomainRules *newDomainRules = new DomainRules(pathOnDisc);//todo implement and ask if this is what DomainRules expects
   domainRulesCache.insert({domain, newDomainRules});
   
   newDomainRules->writeRulesToDisc();
   }

DirectoryRules *RobotsTxt::CreateDirectoryRules(char *directoryName, 
   vector<int> &childIndices, bool isAllowed, bool hasRule)
   {
   string dirNameStr(directoryName);
   DirectoryRules *newDirRule = new DirectoryRules(dirNameStr, isAllowed, hasRule);
   newDirRule->SetChildIndices(childIndices);
   }

void RobotsTxt::ReadRulesFromDisc(FILE *file, vector<DirectoryRules*> &rules)
   {
   char curChar = (char)fgetc(file);
   while(curChar != EOF
       {
       //get name of directory
       char dirName[99];
       int dirNameInd = 0;

       while(curChar != ' ')
       {
       dirName[dirNameInd++] = curChar;
       curChar = (char)fgetc(file):
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

   vector<DirectoryRule*> rules;
   ReadRulesFromDisc(file, rules);

   fclose(fileName.c_str());

   //convert to DomainRules object and place in cache
   CreateDirectoryRuleTree(rules);
   if(rules.empty())
      {
      printf("No root folder!");
      return false;
      }

   DirectoryRules *root = rules[0];
   DomainRules *newRule = new DomainRules(root);
   domainRulesCache.insert(domain, newRule);
   } 

bool RobotsTxt::GetRule(string &path, string &domain)
   { 
   //Look in cache
   if(domainRulesCache.find(domain) != domainRulesCache.end())
      {
      DomainRules *domainRule = domainRulesCache[domain];
      return domainRule->IsAllowed(path);
      }

   //Search in disc
   else
      {
      bool transferSuccess = TransferRulesFromDiscToCache(domain);
      if(!transferSucccess)
         {
         printf("This domain is not on file!");
         throw(1);
         }
      if(domainRulesCache.find(domain) != domainRulesCache.end())
          {
          DomainRules *domainRule = domainRulesCache[domain];
          return domainRule->IsAllowed(path);
          }
      else
         {
         printf("Something's wrong with transfering from disc to cache!");
         throw(1);
         }
      }
   }
=======
/* Created on 2/18, wrote function outlines for constructor/helpers
 * Updated on 2/19, fixed compile issues
 */
#include <unistd.h>
#include "String.h"
#include "TokenStream.h"
#include "RobotsTxt.h"

String UserAgentName_G("*");
String UserAgentCommand_G("User-agent:");
String AllowCommand_G("Allow:");
String DisallowCommand_G("Disallow:");

struct Rule {
	String path;
	bool allow;

	operator bool() const { return !path.Empty(); }
};

bool FindUserAgentRules(String&, TokenStream&);
//Rule ReadNextRule(TokenStream&);

// TODO: Add in TwoBufferFileReader exception to conditions
RobotsTxt::RobotsTxt(const char* robotsFilename) {
	TokenStream robotsReader(robotsFilename);
	
	if(!robotsReader || !FindUserAgentRules(UserAgentName_G, robotsReader))
		return;
	
	//while(Rule curRule = ReadNextRule(robotsReader))
	//	AddRule(curRule);
}

void RobotsTxt::AddRule(Rule rule) {
	// Follow Allow/Disallow rules in RobotsPseudoCode
}

bool FindUserAgentRules(String& userAgent, TokenStream& tokenStream) {
	while(true) {
		if(
		   tokenStream.MatchKeyword(UserAgentCommand_G) && 
		   tokenStream.DiscardWhitespace() &&
		   tokenStream.MatchKeyword(UserAgentName_G) && 
		   (tokenStream.DiscardWhitespace() || true) &&
		   tokenStream.MatchEndline()
		)
			return true;

		if(!tokenStream.SkipLine())
			return false;
	}

	return false;
}

/*
Rule ReadNextRule(TokenStream& tokenStream) {
	try{
		String toMatch(DisallowCommand_G);
		String path;

		// End of User-Agent Rules
		if(fileReader.Peek() == '\n')
			return {String(), false};

		if(fileReader.Peek() == AllowCommand_G[0])
			toMatch = AllowCommand_G;

		// Match Allow or Disallow Rule
		if(Match(toMatch, fileReader) &&
				MatchWhitespaceKleene(fileReader) &&
				MatchPath(path, fileReader) &&
				MatchWhitespaceKleene(fileReader) &&
				fileReader.GetNextCharacter() == '\n')
			return {path, toMatch == AllowCommand_G ? true : false};

		// Malformed Rule
		MoveToNextLine(fileReader);
	} catch(...) {
		
	}

	return {String(), false};
}
*/
>>>>>>> robots
