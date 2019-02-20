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

bool FindUserAgentRules(String, TwoBufferFileReader&);
Rule ReadNextRule(TwoBufferFileReader&);

bool MatchWhitespaceKleene(TwoBufferFileReader&);
bool Match(string, TwoBufferFileReader&);
bool MatchPath(string&, TwoBufferFileReader&);
void MoveToNextLine(TwoBufferFileReader&);

// TODO: Add in TwoBufferFileReader exception to conditions
RobotsTxt::RobotsTxt(String robotsFilename) {
	TwoBufferFileReader robotsReader(robotsFilename.CString());
	
	if(!robotsReader || !FindUserAgentRules(UserAgentName_G, robotsReader))
		return;
	
	while(Rule curRule = ReadNextRule(robotsReader))
		AddRule(curRule);
}

void RobotsTxt::AddRule(Rule rule) {
	// Follow Allow/Disallow rules in RobotsPseudoCode
}

bool FindUserAgentRules(String userAgent, TwoBufferFileReader& fileReader) {
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
		String toMatch = DisallowCommand_G;
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
