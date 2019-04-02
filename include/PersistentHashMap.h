// Created by Graham Eger on 4/1/2019

#pragma once
#ifndef PERSISTENTHASHMAP_H_398
#define PERSISTENTHASHMAP_H_398

#include <cstdlib>
#include <cstdio>

#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "Pair.h"
#include "String.h"
#include "PersistentBitVector.h"

template <typename Key, typename Mapped>
class PersistentHashMap {
public:

    // Custom typedefs
    using KeyType = Key;
    using MappedType = Mapped;
    using ValueType = Pair<Key, Mapped>;
    using SizeType = std::size_t;

    // Constructors
    template <typename InputIterator>
    PersistentHashMap(String filename, InputIterator begin, InputIterator end,
    double loadFactorIn = 0.7);
    PersistentHashMap(String filename, double loadFactorIn = 0.7);

    // Square Brackets Operator
    MappedType& operator[](const KeyType& key);
    MappedType& at(const KeyType& key);

    // Inserts value into hash table, thread safe.
    void insert(const ValueType& keyVal);

    // Iterator forward declares
    class Iterator;
    friend class Iterator;
    Iterator find(const KeyType& key);

    // Erase the item in the table matching the key. 
    // Returns whether an item was deleted.
    bool erase(const KeyType& key);

    // Usual iterator operations
    Iterator begin();
    Iterator end();

    // Basic info member functions
    SizeType size() const;
    bool empty() const;
    SizeType capacity() const;

private:
    static const SizeType INITIAL_CAPACITY = 16;
    void printState() const;

    // probing functions
    SizeType probeForExistingKey(const KeyType& key ) const;
    SizeType probeEmptySpotForKey(const KeyType& key ) const;

    // insert funcitons
    void insertKeyValue(const ValueType& value);
    void noProbeNoRehashInsertKeyValueAtIndex(const ValueType &value, SizeType index);

    // TODO: make this work
    void rehashAndGrow();

    bool rehashNeeded() const;

    // private data
    struct HeaderType {
        size_t numElements;
        size_t capacity;
        double loadFactor;
        threading::ReadWriteLock rwLock;
    };
    int fd; 
    HeaderType * header;
    ValueType * buckets;
    PersistentBitVector isGhost;
    PersistentBitVector isFilled;

};



template <typename Key, typename Mapped> PersistentHashMap<Key, Mapped>::PersistentHashMap(String filename, double loadFactorIn) : isGhost(filename + String("_ghost")), isFilled(filename + String("_filled")) {
    // check if the file exists
    struct stat buffer;
    bool fileExists = (stat(filename.CString(), &buffer) == 0);

    // open file with correct flags
    int openFlags = O_RDWR;
    if (!fileExists) {
        openFlags |= O_CREAT;
    }
    fd = open(filename.CString(), openFlags);
    if (fd < 0) {
        fprintf(stderr, "open() returned -1 - error: %s\n", strerror(errno));
        // TODO: more error handling
    }

    // mmap and setup the header portion
    header = (HeaderType*)mmapWrapper(fd, sizeof(HeaderType), 0);
    header->rwLock = threading::ReadWriteLock();
    if (!fileExists) {
        header->capacity = INITIAL_CAPACITY;
    }

    // mmap the data portion
    buckets = (ValueType*)mmapWrapper(fd, header->capacity * sizeof(ValueType), sizeof(HeaderType));
}

#endif