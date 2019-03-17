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

bool String::Compare(const String& other) const {
	if(size != other.size) return false;

	int index = 0, i;
	while((i = index++) != size && cstring[i] == other.cstring[i]) {}

	return index > size;
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
