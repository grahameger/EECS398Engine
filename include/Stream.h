// Created by Graham Eger on 04/07/2019
// Abstraction of a Stream that's truly created from a number of larger files.

#pragma once
#ifndef STREAM_398_H
#define STREAM_398_H

#include <cstddef>
#include <unistd.h>
#include <string>
#include <map>

struct FileSystem;

class Stream {
public:
    // constructor
    // Creates directory and first backing file if the specified one does not exist.
    // Otherwise, reads all the backing files and populates member variables accordingly.
    Stream();
    // destructor
    ~Stream();
    // copy constructor = delete
    Stream(const Stream& other) = delete;
    // no move semantics

    // writes len bytes at src and returns the starting offset
    // borrows src
    // blocking function, returns -1 upon error
    ssize_t write(char * src, size_t len);

    // returns a pointer to memory which coresponds with
    // the given offset.
    // Accessing offsets which have not been written to is 
    // Undefined behavior
    // May block if offset is currently unmapped
    char * read(size_t offset);

    size_t size();
    size_t capacity();
    //static const size_t BACKING_FILE_SIZE = 1024 * 1024 * 1024; // 10 GiB
    static const size_t BACKING_FILE_SIZE = 4096 * 16 * 16 * 16; // 16 MiB 

private:
    friend struct FileSystem;
    struct BackingFile {
        size_t fileSize; // the number of bytes actually written to the file
    };
    void allocateNewFile();
    bool inArena(char* ptr, size_t fileNo);
    std::map<size_t, BackingFile*> backingFiles; // sorted map
    size_t totalSize;
    inline static const char STREAM_DIRECTORY_NAME[] = "backingFiles";
};

#endif