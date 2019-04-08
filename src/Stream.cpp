// Created by Graham Eger on 04/07/2019
// Abstraction of a Stream backed by a number of files on the actual filesystem.

#include <string>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <cstring>

#include "Stream.h"
#include "mmap.h"

// constructor
// Creates directory and first backing file if one does not exist
// Otherwise, reads all the backing files 
Stream::Stream() {
    totalSize = 0;
    // Check if the directory exists.
    struct stat st;
    char pathname[1024];
    // TODO: check the ordering of the files that are created.
    if (stat(STREAM_DIRECTORY_NAME, &st) && S_ISDIR(st.st_mode)) {
        // for loop through each file in the directory
        DIR * dir;
        struct dirent * entry;
        if ((dir = opendir(STREAM_DIRECTORY_NAME)) != nullptr) {
            // open all of the files within the directory
            while ((entry = readdir(dir)) != nullptr) {
                // there should be no subdirectories, we will ignore them regardless
                int fd = 0;
                if (entry->d_type == DT_REG || entry->d_type == DT_LNK) {
                    // build the pathname
                    sprintf(pathname, "%s/%s", STREAM_DIRECTORY_NAME, entry->d_name);
                    // open the file with read write permissions
                    fd = open(pathname, O_RDWR);
                    // map the file
                    BackingFile * mapping = (BackingFile*)streamMmapWrapper(fd, BACKING_FILE_SIZE);
                    totalSize += mapping->fileSize;
                    // instead of pushing back we're going to make it the index of atoi(filename)
                    // all of the filenames 
                    size_t fileNumber = std::atoi(entry->d_name);
                    backingFiles.insert({fileNumber, mapping});
                }
            }
        }
        // if all files are full
        if (backingFiles.rbegin()->second->fileSize == BACKING_FILE_SIZE) {
            allocateNewFile();
        }

    } else {
        // create and open a new file in the directory
        sprintf(pathname, "%s/%c", STREAM_DIRECTORY_NAME, '1');
        int fd = open(pathname, O_RDWR | O_CREAT);
        // map the file
        BackingFile * mapping = (BackingFile*)streamMmapWrapper(fd, BACKING_FILE_SIZE);
        // add it to the map
        backingFiles.insert({1, mapping});
        mapping->fileSize = sizeof(BackingFile);
        totalSize += sizeof(BackingFile);
    }
}

Stream::~Stream() {
    // unmap all of the files
    for (size_t i = 0; i < backingFiles.size(); ++i) {
        munmapWrapper(backingFiles[i], BACKING_FILE_SIZE);
    }
    // that's it!!!
}

ssize_t Stream::write(char * src, size_t len) {
    size_t spaceLeftOnLast = BACKING_FILE_SIZE - backingFiles.rbegin()->second->fileSize;
    if (spaceLeftOnLast < len) {
        allocateNewFile();
    }
    // do the copy
    BackingFile * lastFile = backingFiles.rbegin()->second;
    void * nextAvailableSpot = lastFile + lastFile->fileSize;
    std::memcpy(nextAvailableSpot, src, len); 
    // increment fileSize 
    lastFile->fileSize += len;
    totalSize += len;
    return 0;
}

char * Stream::read(size_t offset) {
    size_t fileNumber = offset / BACKING_FILE_SIZE;
    if (fileNumber > backingFiles.size() - 1) {
        return nullptr;
    }
    void * basePtr = backingFiles[fileNumber];
    size_t locationWithinFile = (offset % BACKING_FILE_SIZE) + sizeof(BackingFile);
    char * rv = (char*)basePtr + locationWithinFile;
    return rv;
}

size_t Stream::size() {
    return totalSize;
}

size_t Stream::capacity() {
    return BACKING_FILE_SIZE * backingFiles.size();
}

void Stream::allocateNewFile() {
    char pathname[1024];
    // open a new file
    std::string filename = std::to_string(backingFiles.size() + 1);
    sprintf(pathname, "%s/%s", STREAM_DIRECTORY_NAME, filename.c_str());
    int fd = open(pathname, O_RDWR | O_CREAT);
    // map it
    BackingFile * mapping = (BackingFile*)streamMmapWrapper(fd, BACKING_FILE_SIZE);
    // add to vector
    backingFiles.insert({backingFiles.size() + 1, mapping});
    // set filesize
    mapping->fileSize = sizeof(BackingFile);
}

bool Stream::inArena(char * ptr, size_t fileNo) {
    char* basePtr = (char*)backingFiles[fileNo];
    return ptr >= basePtr + 1 && ptr < basePtr + BACKING_FILE_SIZE;
}