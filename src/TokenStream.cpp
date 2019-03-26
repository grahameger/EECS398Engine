#include <fcntl.h>
#include <unistd.h>
#include "String.h"
#include "List.h"
#include "TokenStream.h"

const int TokenStream::BufferSize = 4096;
const char WhitespaceCharacters[] = {' ', '\t', '\r', 0};

bool IsWhitespace(const char toCheck) {
	for(int i = 0; WhitespaceCharacters[i] != 0; i++) {
		if(WhitespaceCharacters[i] == toCheck)
			return true;
	}

	return false;
}

/* ### PUBLIC METHODS ### */

TokenStream::TokenStream(const char* filename) 
: fileDescriptor(open(filename, O_RDONLY)), lexemeStart(0), 
peekIndex(0), lastIndex(-1)
{
	front = buffers.GetFront();
	back = buffers.GetBack();
}

TokenStream::~TokenStream() {
	while(!buffers.Empty()) { delete buffers.RemoveBack(); }
}

bool TokenStream::MatchKeyword(const String& keyword) {
	for(int i = 0; i < keyword.Size(); i++) {
		if(PeekNext() == keyword[i])
			continue;
		ResetPeek();
		return false;
	}

	ConsumeLexeme();
	return true;
}

bool TokenStream::DiscardWhitespace() {
	bool returnValue = false;
	while(IsWhitespace(PeekNext())) { returnValue = true; }
	DecrementPeek();
	ConsumeLexeme();
	return returnValue;
}

bool TokenStream::MatchEndline() {
	switch(PeekNext()) {
		case '\n':
			break;
		case '\r':
			if(PeekNext() == '\n')
				break;
		default:
			ResetPeek();
			return false;
	}

	ConsumeLexeme();
	return true;
}

bool TokenStream::SkipLine() {
	int c = PeekNext();
	for( ; c != '\n' && c >= 0; c = PeekNext()) {}
	ConsumeLexeme();
	return c >= 0;
}

TokenStream::operator bool() const {
	return fileDescriptor != -1 && lastIndex == -1;
}

/* ### PRIVATE METHODS */

void TokenStream::AddPage() {
	if(back != buffers.GetBack()) { ++back; return; }

	char* newBuffer = new char[BufferSize];
	int bytesRead = read(fileDescriptor, newBuffer, BufferSize);
	if(bytesRead != BufferSize)
		lastIndex = bytesRead;
	
	buffers.AddToBack(newBuffer);
	back = buffers.GetBack();
	front = buffers.GetFront();
}

const int TokenStream::PeekNext() {
	if(peekIndex % BufferSize == 0 && (peekIndex != 0 || buffers.Empty()))
		AddPage();
	
	if(peekIndex % BufferSize == lastIndex && back == buffers.GetBack())
		return -1;
	
	return back[peekIndex++ % BufferSize];
}

void TokenStream::ResetPeek() {
	peekIndex = lexemeStart;
	back = front;
}

void TokenStream::DecrementPeek() {
	if(peekIndex == lexemeStart)
		return;

	if(peekIndex-- % BufferSize == 0)
		back--;
}

void TokenStream::ConsumeLexeme() {
	lexemeStart = peekIndex;
	while(buffers.GetFront() != back) { delete buffers.RemoveFront(); }
	front = back;
}
