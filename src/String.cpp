#include <cstring>
#include "String.h"

const char* String::nullString = "";

String::String() : cstring(nullptr), size(0) {}

String::String(const char* toCopy) : size(strlen(toCopy)) {
	cstring = new char[size + 1];
	strcpy(cstring, toCopy);
}

String::String(char*&& toMove) : cstring(toMove), size(strlen(toMove)) {
	toMove = nullptr;
}

String::~String() {
	delete[] cstring;
	cstring = nullptr;
}

bool String::Empty() const { return size == 0; }
int String::Size() const { return size; }

const char* String::CString() const {
	return cstring == nullptr ? nullString : cstring;
}

const char String::operator[] (int index) const {
	if(cstring == nullptr)
		return nullString[index];

	return cstring[index];
}

char& String::operator[] (int index) {
	if(cstring == nullptr){
		cstring = new char[1];
		cstring[0] = 0;
	}

	return cstring[index];
}
