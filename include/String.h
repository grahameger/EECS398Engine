#ifndef STRING_H
#define STRING_H

class String {
public:
	String();
	String(const char* toCopy);
	String(char*&& toMove);
	~String();

	bool Empty() const;
	int Size() const;
	const char* CString() const;
	bool Compare(const String& other) const;

	const char operator[] (int index) const;
	char& operator[] (int index);

private:
	const static char* nullString;
	char* cstring;
	int size;
};

#endif
