#ifndef TWOBUFFERFILEREADER_H
#define TWOBUFFERFILEREADER_H

#define BUFFERSIZE 4096

class TwoBufferFileReader {
public:
	TwoBufferFileReader(char* filename);
	const char GetNextCharacter();
	const char Peek();

private:
	char[BUFFERSIZE + 1] buffer1;
	char[BUFFERSIZE + 1] buffer2;
};

#endif
