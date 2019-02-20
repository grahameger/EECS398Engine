/* Created on 2/18, wrote function outlines for constructor/helpers
 * Updated on 2/19, fixed compile issues
 */
#include <unistd.h>
#include "RobotsTxt.h"
#include "TwoBufferFileReader.h"

using std::string;

string UserAgentName_G("*");
string UserAgentCommand_G("User-agent:");
string AllowCommand_G("Allow:");
string DisallowCommand_G("Disallow:");

struct Rule {
	string path;
	bool allow;

	operator bool() const { return !path.empty(); }
};

bool FindUserAgentRules(string, TwoBufferFileReader&);
Rule ReadNextRule(TwoBufferFileReader&);
bool MatchWhitespaceKleene(TwoBufferFileReader&);
bool Match(string, TwoBufferFileReader&);
bool MatchPath(string&, TwoBufferFileReader&);
void MoveToNextLine(TwoBufferFileReader&);

// TODO: Add in TwoBufferFileReader exception to conditions
RobotsTxt::RobotsTxt(string robotsFilename) {
	TwoBufferFileReader robotsReader(robotsFilename.c_str());
	
	if(!robotsReader || !FindUserAgentRules(UserAgentName_G, robotsReader))
		return;
	
	while(Rule curRule = ReadNextRule(robotsReader))
		AddRule(curRule);
}

void RobotsTxt::AddRule(Rule rule) {
	// Follow Allow/Disallow rules in RobotsPseudoCode
}

bool FindUserAgentRules(string userAgent, TwoBufferFileReader& fileReader) {
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

			MoveToNextLine(fileReader);
		}
	} catch(...) {
		// Do Nothing
	}

	return false;
}

Rule ReadNextRule(TwoBufferFileReader& fileReader) {
	try{
		string toMatch = DisallowCommand_G;
		string path;

		// End of User-Agent Rules
		if(fileReader.Peek() == '\n')
			return {string(), false};

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

	return {string(), false};
}

void MoveToNextLine(TwoBufferFileReader& fileReader) {
	MatchWhitespaceKleene(fileReader);
	while(fileReader.GetNextCharacter() != '\n') {}
}

bool Match(string toMatch, TwoBufferFileReader& fileReader) {
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
