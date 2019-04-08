// Created by Graham Eger on 04/07/2019

// Abstraction for a File that lives within a Stream.
// Necessary as to not create billions of .html files in our ext4 RAID5 volume.

#include "File.h"
#include <cstring>

// load unaligned uint64_t from the pointer V
static uint64_t load64LittleEndian(char const* V)
{
   uint64_t Ret = 0;
   Ret |= (uint64_t) V[0];
   Ret |= ((uint64_t) V[1]) << 8;
   Ret |= ((uint64_t) V[2]) << 16;
   Ret |= ((uint64_t) V[3]) << 24;
   Ret |= ((uint64_t) V[4]) << 32;
   Ret |= ((uint64_t) V[5]) << 40;
   Ret |= ((uint64_t) V[6]) << 48;
   Ret |= ((uint64_t) V[7]) << 56;
   return Ret;
}
// TODO: finish replacing the alignment stuff

// disk will have already been initialized at this point
FileSystem::FileSystem() {
    m = PTHREAD_MUTEX_INITIALIZER;
    // Iterate through each backing file
    for (size_t i = 0; i < disk.backingFiles.size(); ++i) {
        char * basePtr = (char*)disk.backingFiles[i] + sizeof(Stream::BackingFile);
        char * c = basePtr;
        while (disk.inArena(c, i)) {
            String filename;
            if (*c != '\0') {
                filename = String(c);
                c += filename.Size() + 1;
            }
            size_t fileSize = *((size_t*)c);
            size_t offset = c - basePtr + i * Stream::BACKING_FILE_SIZE;
            mapping.insert({filename, offset});
            c += fileSize + sizeof(size_t);
        }
    }
}

// File stuff
File::File(const char * filename, void * src, size_t len) {
    pthread_mutex_lock(&fs.m);
    // create a temporary buffer to move everything into
    const size_t filenameSize = strlen(filename) + 1;
    const size_t totalSize = filenameSize + len + sizeof(size_t);
    char * temp = (char*)malloc(totalSize);
    // copy everything over
    std::memcpy(temp, filename, filenameSize);
    std::memcpy(temp + filenameSize, &len, sizeof(size_t));
    std::memcpy(temp + filenameSize + sizeof(size_t), src, len);
    // write it to the stream
    offset = fs.disk.write(temp, totalSize);
    lenOffset = offset + filenameSize;
    // free the temp buffer
    free(temp);
    // insert the mapping
    if (offset != -1) {
        fs.mapping.insert({filename, offset});
    }
    // add it to our hash table
    pthread_mutex_unlock(&fs.m);
}

File File::find(const char * filename) {
    auto i = fs.mapping.find(filename);
    File f;
    if (i == fs.mapping.end()) {
        f.offset = npos;
        f.lenOffset = npos;
    } else {
        f.offset = i->second;
        f.lenOffset = strlen(fs.disk.read(f.offset)) + 1;
    }
    return f;
}

size_t File::size() {
    return load64LittleEndian(fs.disk.read(lenOffset));
}

FileRead File::read() {
    FileRead rv;
    char * lenPtr = fs.disk.read(lenOffset);
    rv.len = load64LittleEndian(lenPtr);
    rv.ptr = lenPtr + sizeof(size_t);
    return rv;
}

String File::name() {
    return fs.disk.read(offset);
}

File::File() : offset(npos), lenOffset(npos) {}
