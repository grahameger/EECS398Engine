#include <unistd.h>
#include "RobotsTxt.h"
#include "DirectoryRules.h"

using std::string_view;

string_view UserAgentName_G("*");
string_view UserAgentCommand_G("User-agent:");
string_view AllowCommand_G("Allow:");
string_view DisallowCommand_G("Disallow:");

struct Rule {
	string_view path;
	bool allow;

	operator bool() const { return path.data; }
};

// TODO: Add in TwoBufferFileReader exception to conditions
RobotsTxt::RobotsTxt(string_view robotsFilename) : root("/", true) {
	TwoBufferFileReader robotsReader(robotsFilename.data);
	
	if(!robotsReader || !FindUserAgentRules(UserAgentName_G, robotsReader))
		return;
	
	while(Rule curRule = ReadNextRule(fileReader))
		AddRule(curRule);
}

void RobotsTxt::AddRule(Rule rule) {
	// Follow Allow/Disallow rules in RobotsPseudoCode
}

bool FindUserAgentRules(string_view userAgent, TwoBufferFileReader& fileReader) {
	try{
		while(true) {
			if(
			   Match(UserAgentCommand_G, fileReader) && 
			   MatchWhitespaceKleene(fileReader) &&
			   Match(UserAgentName_G, fileReader) && 
			   MatchWhitespaceKleene(fileReader) &&
			   fileReader.GetNextCharacter() == '\n'
			)
				return true;

			MoveToNextLine();
		}
	} catch( ) {
		// Do Nothing
	}

	return false;
}

Rule ReadNextRule(TwoBufferFileReader& fileReader) {
	try{
		string_view toMatch = DisallowCommand_G;
		string_view path;

		// End of User-Agent Rules
		if(fileReader.Peek() == '\n')
			return {string_view(), false};

		if(fileReader.Peek() == AllowCommand_G[0])
			toMatch = AllowCommand_G;

		// Match Allow or Disallow Rule
		if(Match(toMatch, fileReader) &&
				MatchWhitespaceKleene(fileReader) &&
				MatchPath(fileReader, path) &&
				MatchWhitespaceKleene(fileReader) &&
				fileReader.GetNextCharacter() == '\n')
			return {path, toMatch == AllowCommand_G ? true : false};

		// Malformed Rule
		MoveToNextLine();
	} catch( ) {
		
	}
}

void MoveToNextLine(TwoBufferFileReader& fileReader) {
	MatchWhitespaceKleene(fileReader);
	while(fileReader.GetNextCharacter() != '\n') {}
}

bool Match(string_view toMatch, TwoBufferFileReader& fileReader) {
	for(int i = 0; i < toMatch.size(); i++) {
		if(fileReader.Peek() != toMatch[i])
			return false;
		fileReader.GetNextCharacter();
	}

	return true;
}

bool MatchWhitespaceKleene(TwoBufferFileReader& fileReader) {
	while(fileReader.Peek() == ' ' || fileReader.Peek() == '\t' ||
	      fileReader.Peek() == '\r')
	   fileReader.GetNextCharacter();
	
	return true;
}
