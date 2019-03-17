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

bool FindUserAgentRules(TokenStream&);
//Rule ReadNextRule(TokenStream&);

// TODO: Add in TwoBufferFileReader exception to conditions
RobotsTxt::RobotsTxt(const char* robotsFilename) {
	TokenStream robotsReader(robotsFilename);
	
	if(!robotsReader || !FindUserAgentRules(robotsReader))
		return;
	
	//while(Rule curRule = ReadNextRule(robotsReader))
	//	AddRule(curRule);
}

void RobotsTxt::AddRule(Rule rule) {
	// Follow Allow/Disallow rules in RobotsPseudoCode
}

bool FindUserAgentRules(TokenStream& tokenStream) {
	while(true) {
		if(MatchedUserAgent(tokenStream))
			return true;

		if(!tokenStream.SkipLine())
			return false;
	}

	return false;
}

bool MatchedUserAgent(TokenStream& tokenStream) {
	return tokenStream.MatchKeyword(UserAgentCommand_G) && 
		   tokenStream.DiscardWhitespace() &&
		   tokenStream.MatchKeyword(UserAgentName_G) && 
		   (tokenStream.DiscardWhitespace() || true) &&
		   tokenStream.MatchEndline();
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
