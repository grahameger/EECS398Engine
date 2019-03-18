#include <fcntl.h>
#include <unistd.h>
#include "String.h"
#include "List.h"
#include "TokenStream.h"

const int TokenStream::BufferSize = 4096;
const char WhitespaceCharacters[] = {' ', '\t', '\r', 0};

bool IsWhitespace(const char toCheck) {
	for(int i = 0; WhitespaceCharacters[i]; i++) {
		if(WhitespaceCharacters[i] == toCheck)
			return true;
	}

	return false;
}

/* ### PUBLIC METHODS ### */

TokenStream::TokenStream(const char* filename) 
: fileDescriptor(open(filename, O_RDONLY)), 
		lexemeStart(0), peekIndex(0), lastIndex(-1)
{
	front = buffers.GetFront();
	back = buffers.GetBack();
}

TokenStream::~TokenStream() 
{
	while(!buffers.Empty()) { delete buffers.RemoveBack(); }
}

void TokenStream::DiscardWhitespace() 
{
	int next;
	while((next = PeekNext()) != -1 && IsWhitespace(next)) {}
	if( next != -1 ) DecrementPeek();
	ConsumeLexeme();
}

bool TokenStream::MatchKeyword(const String& keyword) 
{
	for(int i = 0; i < keyword.Size(); i++) {
		if(PeekNext() == keyword[i])
			continue;
		ResetPeek();
		return false;
	}

	ConsumeLexeme();
	return true;
}

bool TokenStream::MatchEndline() 
{
	switch(PeekNext()) {
		case -1:
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

String TokenStream::MatchPath()
{
	if(PeekNext() != '/')
	{
		ResetPeek();
		return String();
	}

	int next;
	while( ( next = PeekNext( ) ) != -1 && 
			!IsWhitespace(next) && next != '\n' ) {}
	if( next != -1 ) DecrementPeek();
	
	return ConsumeLexemeToString();
}

bool TokenStream::MatchNextKeyword(const String& keyword) {
	int next;
	while((next = PeekNext()) != -1) 
	{
		if(next != keyword[0]) continue;

		int i;
		for(i = 1; i < keyword.Size(); i++)
		{
			if((next = PeekNext()) != keyword[i]) break;
		}

		if(next == -1) break;

		if(i >= keyword.Size())
		{
			ConsumeLexeme();
			return true;
		}

		for(int j = 0; j < i; j++)
			DecrementPeek();
	}

	return false;
}

bool TokenStream::MatchNextEndline() {
	int next;
	while((next = PeekNext()) != -1 && next != '\n') {}
	return next != -1;
}

TokenStream::operator bool() const 
{
	return fileDescriptor != -1 && lastIndex != peekIndex % BufferSize;
}

/* ### PRIVATE METHODS ### */

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

String TokenStream::ConsumeLexemeToString() {
	int length = peekIndex - lexemeStart;
	if(!length)
		return String();
	char* cstring = new char[length + 1];

	for(int i = 0, pages = 1; i < peekIndex - lexemeStart; i++) {
		if( i < BufferSize*pages - lexemeStart % BufferSize )
			cstring[ i ] = front[ (lexemeStart + i) % BufferSize ];
		else {
			front++;
			delete buffers.RemoveFront();
			pages++;
		}
	}

	cstring[length] = 0;
	lexemeStart = peekIndex;

	return String(cstring);
}
