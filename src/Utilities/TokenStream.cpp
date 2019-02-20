#include <fcntl.h>
#include <unistd.h>
#include "String.h"
#include "List.h"
#include "TokenStream.h"

const int TokenStream::BufferSize = 4096;

/* ### PUBLIC METHODS ### */

TokenStream::TokenStream(const char* filename) 
: fileDescriptor(open(filename, O_RDONLY)), lexemeStart(0), peekIndex(0) {
	front = buffers.GetFront();
	back = buffers.GetBack();
}

TokenStream::~TokenStream() {
	while(!buffers.Empty()) { delete buffers.RemoveBack(); }
}

bool TokenStream::MatchKeyword(const String keyword) {
	for(int i = 0; i < keyword.Size(); i++) {
		if(PeekNext() == keyword[i])
			continue;
		ResetPeek();
		return false;
	}

	ConsumeLexeme();
	return true;
}

TokenStream::operator bool() const {
	return fileDescriptor != -1;
}

/* ### PRIVATE METHODS */

void TokenStream::AddPage() {
	if(back != buffers.GetBack()) { ++back; return; }

	char* newBuffer = new char[BufferSize];
	if(read(fileDescriptor, newBuffer, BufferSize * sizeof(char)) != BufferSize)
		fileDescriptor = -1;
	
	buffers.AddToBack(newBuffer);
	++back;
}

const char TokenStream::PeekNext() {
	if(peekIndex % BufferSize == 0)
		AddPage();
	
	return back[peekIndex++ % BufferSize];
}

void TokenStream::ResetPeek() {
	peekIndex = lexemeStart;
	while(back != front) { back--; }
}

void TokenStream::ConsumeLexeme() {
	lexemeStart = peekIndex;
	while(buffers.GetFront() != back) { delete buffers.RemoveFront(); }
	front = back;
}
