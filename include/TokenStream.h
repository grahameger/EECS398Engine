#ifndef TOKENSTREAM_H
#define TOKENSTREAM_H

#include "List.h"

class TokenStream {
public:
	TokenStream(const char* filename);
	~TokenStream();

	TokenStream(const TokenStream&) = delete;
	void operator=(const TokenStream&) = delete;

	bool MatchKeyword(const String keyword);

	operator bool() const;

private:
	const static int BufferSize;

	int fileDescriptor, lexemeStart, peekIndex;
	List<char*> buffers;
	List<char*>::Iterator front, back;

	void AddPage();
	void ResetPeek();
	void ConsumeLexeme();
	const char PeekNext();
};

#endif
