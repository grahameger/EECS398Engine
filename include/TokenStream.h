#ifndef TOKENSTREAM_H
#define TOKENSTREAM_H

#include "List.h"

class TokenStream {
public:
	TokenStream(const char* filename);
	~TokenStream();

	TokenStream(const TokenStream&) = delete;
	void operator=(const TokenStream&) = delete;

	bool MatchKeyword(const String& keyword);
	bool DiscardWhitespace();
	bool MatchEndline();

	bool SkipLine();

	operator bool() const;

private:
	const static int BufferSize;

	int fileDescriptor, lexemeStart, peekIndex, lastIndex;
	List<char*> buffers;
	List<char*>::Iterator front, back;

	void AddPage();
	void ResetPeek();
	void DecrementPeek();
	void ConsumeLexeme();
	const int PeekNext();
};

#endif
