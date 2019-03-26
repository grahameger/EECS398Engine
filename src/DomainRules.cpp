/* Created on 2/18, wrote function outlines for constructor/helpers, author: Jason
 * Updated on 2/19, fixed compile issues, author: Jason
 * Updated on 3/18, rewrote FindUserAgentRules, author: Jason
 * Updated on 3/20, added new constructor and WriteToDisc(), author: Dennis
 */

#include <iostream>
#include <unistd.h>
#include "RobotsTxt.h"
#include "String.h"
#include "TokenStream.h"
#include "DirectoryRules.h"

String UserAgentName_G("*");
String UserAgentCommand_G("User-agent:");
String AllowCommand_G("Allow:");
String DisallowCommand_G("Disallow:");

struct Rule {
	String path;
	bool allow;

	operator bool() const { return !path.Empty(); }
};

bool FindUserAgentRules(TokenStream&);
Rule ReadNextRule(TokenStream&);

/* ### PUBLIC METHODS ### */
DomainRules::DomainRules(DirectoryRules* rootIn)
	: root(rootIn) {}

DomainRules::DomainRules(const char* robotsFilename)
{
	TokenStream robotsReader(robotsFilename);
	
	if(!robotsReader || !FindUserAgentRules(robotsReader))
		return;

	#ifdef TEST
	std::cout << "User agent rules found" << std::endl;
	#endif
	
	while(Rule curRule = ReadNextRule(robotsReader))
		AddRule(curRule);
}

void DomainRules::AddRule(Rule rule) {
	// Follow Allow/Disallow rules in RobotsPseudoCode
}

/* ### HELPER FUNCTIONS ### */

bool FindUserAgentRules(TokenStream& tokenStream) {
	while(true) {
		if(!tokenStream.MatchNextKeyword(UserAgentCommand_G))
			return false;
		tokenStream.DiscardWhitespace();
		if(!tokenStream.MatchKeyword(UserAgentName_G))
			continue;
		tokenStream.DiscardWhitespace();
		if(tokenStream.MatchEndline())
			return true;
	}

	return false;
}

Rule ReadNextRule(TokenStream& tokenStream) {
	while(true)
	{
		if(tokenStream.MatchEndline())
			return {String(), false};

		bool allowed;
		if(tokenStream.MatchKeyword(DisallowCommand_G)) 
			allowed = false;
		else if(tokenStream.MatchKeyword(AllowCommand_G))
			allowed = true;
		else
		{
			tokenStream.MatchNextEndline();
			continue;
		}

		tokenStream.DiscardWhitespace();

		String path = tokenStream.MatchPath();
		tokenStream.MatchNextEndline();

		if(!path) continue;

		#ifdef TEST
		std::cout << "Rule Found: " << path.CString();
		std::cout << " is " << (allowed ? "allowed" : "disallowed") << std::endl;
		#endif
		return {path, allowed};
	}

	return {String(), false};
}

void DomainRules::WriteRulesToDisc(std::string &domain) {
	FILE *file = fopen(domain.c_str(), "w");
	root->SaveToFile(file);
	fclose(file);
}


