// Created by Graham Eger on 04/07/2019

// Abstraction for a File that lives within a Stream.
// Necessary as to not create billions of .html files in our ext4 RAID5 volume.

#include "File.h"
#include <cstring>
#include <cassert>

// load unaligned uint64_t from the pointer p
static uint64_t load64LittleEndian(char const* p)
{
    size_t rv;
    std::memcpy(&rv, p, sizeof(uint64_t));
    return rv;
}

// disk will have already been initialized at this point
FileSystem::FileSystem() {
    // iterate through each backing file
    std::map<size_t, Stream::BackingFile*>::iterator i;
    for ( i = disk.backingFiles.begin(); i != disk.backingFiles.end(); i++) {
        char * basePtr = ((char*)i->second + sizeof(Stream::BackingFile));
        char * c = basePtr;
        while (disk.inArena(c, i->first) && (c - basePtr) < i->second->fileSize) {
            String filename;
            if (*c != '\0') {
                filename = String(c);
                c += filename.Size() + 1;
            }
            size_t fileSize = load64LittleEndian(c);
            size_t offset = c - basePtr + (i->first - 1) * Stream::BACKING_FILE_SIZE;
            mapping.insert({filename, offset});
            c += fileSize + sizeof(size_t);
        }
    }
    m = PTHREAD_MUTEX_INITIALIZER;
}

// File stuff
File::File(const char * filename, void * src, size_t len) {
    pthread_mutex_lock(&fs.m);
    // create a temporary buffer to move everything into
    const size_t filenameSize = strlen(filename) + 1;
    const size_t totalSize = filenameSize + len + sizeof(size_t);
    char * temp = (char*)malloc(totalSize);
    // copy everything over
    memcpy(temp, filename, filenameSize);
    memcpy(temp + filenameSize, (void*)&len, sizeof(size_t));
    memcpy(temp + filenameSize + sizeof(size_t), src, len);

    // write it to the stream
    offset = fs.disk.write(temp, totalSize);
    lenOffset = offset + filenameSize;
    // free the temp buffer
    free(temp);
    // insert the mapping
    if (offset != -1) {
        fs.mapping.insert({filename, offset});
        fprintf(stdout, "created file %s at offset %zu\n", filename, offset);
    }
    // add it to our hash table
    pthread_mutex_unlock(&fs.m);
}

File File::find(const char * filename) {
    File f;
    if (filename) {
        pthread_mutex_lock(&fs.m);
        std::unordered_map<String, size_t>::iterator i = fs.mapping.find(filename);
        if (i == fs.mapping.end()) {
            pthread_mutex_unlock(&fs.m);
            f.offset = npos;
            f.lenOffset = npos;
        } else {
            f.offset = i->second;
            const char * filenamePtr = fs.disk.read(f.offset);
            pthread_mutex_unlock(&fs.m);
            size_t filenameLength = strlen(filenamePtr);
            f.lenOffset = f.offset + filenameLength + 1;
        }
    }
    return f;
}

size_t File::size() {
    pthread_mutex_lock(&fs.m);
    size_t rv = load64LittleEndian(fs.disk.read(lenOffset));
    pthread_mutex_unlock(&fs.m);
    return rv;
}

FileRead File::read() {
    FileRead rv;
    pthread_mutex_lock(&fs.m);
    char * lenPtr = fs.disk.read(lenOffset);
    pthread_mutex_unlock(&fs.m);
    rv.len = load64LittleEndian(lenPtr);
    rv.ptr = lenPtr + sizeof(size_t);
    return rv;
}

String File::name() {
    pthread_mutex_lock(&fs.m);
    String rv(fs.disk.read(offset));
    pthread_mutex_unlock(&fs.m);
    return rv;
}

File::File() : offset(npos), lenOffset(npos) {}

bool File::exists() {
    return offset != npos && lenOffset != npos;
}
