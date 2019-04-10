// Created by Graham Eger on 04/07/2019

// Abstraction for a File that lives within a Stream.
// Necessary as to not create billions of .html files in our ext4 RAID5 volume.

#pragma once
#ifndef FILE_398_H
#define FILE_398_H

#include <unordered_map>

#include <pthread.h>

#include "Stream.h"
#include "String.h"
#include "hash.h"

struct FileSystem {
    
    FileSystem();
    // everything else is default, just using this to initially populate hash table
    std::unordered_map<String, size_t> mapping; // TODO: use our hash table
    Stream disk;
    pthread_mutex_t m;
};

struct FileRead {
    char * ptr;
    size_t len;
};

// Trivially copyable, somewhat serializable, references the static members
// The mapping is initialized by reading through the entire stream.
// Files are arranged on the stream like so
// [null-terminated filename][size_t fileSize][filecontents of length fileSize][...][...][...]
class File {
public:
    // constructor
    File();
    File(const char * filename, void * src, size_t len);
        // pass a filename, pointer, and length
        // creates and writes to disk atomically
    // destructor default 
    // copy constructor default 
    // move constructor default 
    // move assignment operator default
    static File find(const char * filename); // goes to the stream
    size_t size();
    static std::pair<size_t, size_t> totalSizeAndNumFiles();
    FileRead read();
    String name();
    bool exists();
private:
    ssize_t offset; // the start of the filename
    ssize_t lenOffset;
    inline static FileSystem fs;
    static const ssize_t npos = -1;
};

#endif