// Created by Graham Eger on 4/1/2019

#pragma once
#ifndef PERSISTENTHASHMAP_H_398
#define PERSISTENTHASHMAP_H_398

#include <cstdlib>
#include <cstdio>
#include <iterator>

#include <iostream>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "hash.h"
#include "mmap.h"
#include "Pair.h"
#include "String.h"
#include "PersistentBitVector.h"

#ifndef O_NOATIME
#define O_NOATIME 0
#endif

// Linear Probing thread-safe file backed hash map
template <typename Key, typename Mapped>
class PersistentHashMap {
public:

    // Custom typedefs
    using KeyType = Key;
    using MappedType = Mapped;
    using ValueType = Pair<Key, Mapped>;
    using SizeType = std::size_t;

    // Constructor
    PersistentHashMap(String filename, double loadFactorIn = 0.7);
    ~PersistentHashMap();

    // Square Brackets Operator
    MappedType& operator[](const KeyType& key);
    MappedType& at(const KeyType& key);

    // Inserts value into hash table, thread safe.
    void insert(const ValueType& keyVal);

    // Flush function
    bool flush();

    // Erase the item in the table matching the key. 
    // Returns whether an item was deleted.
    bool erase(const Key& key);

    // Iterator forward declares
    class Iterator;
    friend class Iterator;
    Iterator find(const KeyType& key);

    // Usual iterator operations
    Iterator begin();
    Iterator end();

    // Basic info member functions
    SizeType size();
    bool empty();
    SizeType capacity();
    void printState();

private:
    static const SizeType INITIAL_CAPACITY = 64;

    // probing functions
    SizeType probeForExistingKey(const KeyType& key );
    SizeType probeEmptySpotForKey(const KeyType& key );

    // insert funcitons
    void insertKeyValue(const ValueType& value);
    void noProbeNoRehashInsertKeyValueAtIndex(const ValueType &value, SizeType index);

    void rehashAndGrow();

    bool rehashNeeded();

    void writeLock();
    void readLock();
    void unlock();
    private:
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

// Iterator class for the hash table, just simple forward iteration.
template <typename Key, typename Mapped>
class PersistentHashMap<Key, Mapped>::Iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Pair<Key, Mapped>;
    using different_type = ssize_t;
    using pointer = std::add_pointer_t<value_type>;
    using reference = std::add_lvalue_reference_t<value_type>;
    Iterator() : index(0) {}
    Iterator(size_t index_in, PersistentHashMap* persistentHashMapIn) : 
            persistentHashMap{persistentHashMapIn}, index{index_in} {}

    // No need for locks here as we already have the index
    // it's on the programmer to know if their iterator has been invalidated
    Pair<Key, Mapped>& operator*() {
        assert(!this->persistentHashMap->isGhost.at(this->index));
        assert(!this->persistentHashMap->isFilled.at(this->index));
        return this->persistentHashMap->buckets[this->index];
    }

    // We'll read lock here just because we're iterating through the Hash Map
    Iterator& operator++() {
        this->persistentHashMap->readLock();
        ++this->index;
        for (; (this->index != this->persistentHashMap->size() &&
                    (this->persistentHashMap->isGhost.at(this->index) ||
                    !this->persistentHashMap->isFilled.at(this->index)));
                ++this->index);
        this->persistentHashMap->unlock();
        return *this;
    }
    
    // No lock necessary, iterators aren't volatile.
    bool operator!= (const Iterator& other) {
        return this->index != other.index;
    }

private:
    PersistentHashMap<Key, Mapped> * persistentHashMap;
    size_t index;
};



template <typename Key, typename Mapped> PersistentHashMap<Key, Mapped>::PersistentHashMap(String filename, double loadFactorIn) : isGhost( filename + "_ghost" ), isFilled( filename + "_filled" ) {
    // check if the file exists
    struct stat buffer;
    bool fileExists = (stat(filename.CString(), &buffer) == 0);

    // open file with correct flags
    int openFlags = O_RDWR | O_NOATIME;
    if (!fileExists) {
        openFlags |= O_CREAT;
    }
    fd = open(filename.CString(), openFlags, 0755);
    if (fd < 0) {
        fprintf(stderr, "open() returned -1 - error: %s\n", strerror(errno));
        exit(1);
    }

    // mmap and setup the header portion
    header = (HeaderType*)mmapWrapper(fd, sizeof(HeaderType), 0);
    // memory mapped files should be initialized to 0
    // jokes on me they're not in practice
    if (header->capacity == 0) {
        header->rwLock = threading::ReadWriteLock();
    }
    // TODO: this feels deadlocky, putting this comment here just in case
    header->rwLock.writeLock();
    if (!fileExists) {
        header->capacity = INITIAL_CAPACITY;
    }
    header->loadFactor = loadFactorIn;
    // mmap the data portion
    this->buckets = (ValueType*)mmapWrapper(fd, header->capacity * sizeof(ValueType), sizeof(HeaderType));
    // we shouldn't memset we should default construct the objects?
    this->header->rwLock.unlock();
}

template <typename Key, typename Mapped> PersistentHashMap<Key, Mapped>::~PersistentHashMap() {
    // unmap buckets
    munmapWrapper(buckets, this->header->capacity * sizeof(ValueType));
    // unmap header
    munmapWrapper(header, sizeof(HeaderType)); 
}

template <typename Key, typename Mapped> bool PersistentHashMap<Key, Mapped>::flush() {
    // sync buckets
    int rv = msyncWrapper(buckets, this->header->capacity * sizeof(ValueType));
    if (rv != 0) {
        return false;
    }
    // sync header
    rv = msyncWrapper(header, sizeof(HeaderType));
    if (rv != 0) {
        return false;
    }
    return true;
}

// Inserts the value into the hash table at a given indice.
template<typename Key, typename Mapped>
void PersistentHashMap<Key, Mapped>::
noProbeNoRehashInsertKeyValueAtIndex(const ValueType &value, SizeType index) {
    // insert into the buckets and then update the other members
    this->buckets[index] = value;
    this->isFilled.set(index, true);
    assert(isFilled.at(index));
    this->isGhost.set(index, false);
    assert(!isGhost.at(index));
}

template<typename Key, typename Mapped>
bool PersistentHashMap<Key, Mapped>::rehashNeeded() {
    double currentLoadFactor = static_cast<double>(this->header->numElements) /
            static_cast<double>(this->header->capacity);
    return currentLoadFactor > this->header->loadFactor;
}

// private insert function
template<typename Key, typename Mapped>
void PersistentHashMap<Key, Mapped>::insertKeyValue(const ValueType& value) {
    // check if the value exists
    auto indexOld = this->probeForExistingKey(value.first);
    if (indexOld != this->header->capacity)
        return;
    
    // Check the load factor for a rehash
    if (this->rehashNeeded())
        this->rehashAndGrow();
    
    // insert the value
    ++this->header->numElements;
    auto indexToInsert = this->probeEmptySpotForKey(value.first);
    this->noProbeNoRehashInsertKeyValueAtIndex(value, indexToInsert);
}

template<typename Key, typename Mapped>
size_t PersistentHashMap<Key, Mapped>::probeEmptySpotForKey(const Key& key) {
    //size_t i = std::hash<KeyType>{}(key) % this->header->capacity;
    size_t i = hash::Hash<KeyType>{}.get(key) % this->header->capacity;
    size_t start = i;
    // when we find one return index, if it already exists, return that index
    for (; i < this->header->capacity; ++i) {
        if (!isFilled.at(i) || isGhost.at(i)) {
            return i;
        }
    }
    for (i = 0; i < start; ++i) {
        if (!isFilled.at(i) || isGhost.at(i)) {
            return i;
        }
    }
    return this->header->capacity;
}

// const functions need a read lock
// non-const functions need a write lock
template<typename Key, typename Mapped>
size_t PersistentHashMap<Key, Mapped>::probeForExistingKey(const Key& key) {
    // if it's there then we return the index
    // otherwise return one past the end
    // size_t i = std::hash<KeyType>{}(key) % this->header->capacity;
    size_t i = hash::Hash<KeyType>{}.get(key) % this->header->capacity;
    size_t start = i;
    for (; (isGhost.at(i) || isFilled.at(i)) && i < this->header->capacity; ++i ) {
        if (buckets[i].first == key) {
            return i;
        }
    }
    if (!isGhost.at(i) && isFilled.at(i)) {
        return this->header->capacity;
    } else {
        for (i = 0; (isGhost.at(i) || isFilled.at(i)) && i < start; ++i) {
            if (buckets[i].first == key) {
                return i;
            }
        }
    }
    // if we get here we should be setting 
    return this->header->capacity;
}


// O(1) average
// O(n) worse case 
template<typename Key, typename Mapped>
Mapped& PersistentHashMap<Key, Mapped>::operator[] (const KeyType& key) {
    // probe for key, if it doesn't exist then insert
    this->writeLock();
    auto indexForKey = this->probeForExistingKey(key);
    if (indexForKey == this->header->capacity) {
        this->insertKeyValue(ValueType{key, MappedType{}});
    }
    // get key and return val at that location
    indexForKey = this->probeForExistingKey(key);
    auto rv = this->buckets[indexForKey].second;
    this->unlock();
    return rv;
}

template<typename Key, typename Mapped>
Mapped& PersistentHashMap<Key, Mapped>::at(const KeyType& key) {
    // probe for key
    this->readLock();
    auto indexForKey = this->probeForExistingKey(key);
    if (indexForKey == this->header->capacity) {
        this->unlock();
        throw std::out_of_range("Key does not exist in hash map");
    }
    auto& rv = this->buckets[indexForKey].second;
    this->unlock();
    return rv;
}

template<typename Key, typename Mapped>
void PersistentHashMap<Key, Mapped>::rehashAndGrow() {
    // double the size of the mapped buckets portion
    size_t newCapacity = this->header->capacity * 2;
    size_t newBucketSize = newCapacity * sizeof(ValueType);

    // unmap old region
    munmapWrapper(buckets, newBucketSize / 2);

    // map the new region
    buckets = (ValueType*)mmapWrapper(fd, newBucketSize, sizeof(HeaderType));
    
    // zero only the new region of memory
    size_t half = sizeof(ValueType) * this->header->capacity;
    memset((void*)((uint8_t*)buckets + half), 0x0, half);

    // double the size of the isFilled and isGhost bit vectors
    isGhost.resize(newCapacity);
    isFilled.resize(newCapacity);

    // adjust the capacity to reflect the changes above
    this->header->capacity = newCapacity;

    // rehash
    for (size_t i = 0; i < newCapacity / 2; ++i) {
        if (!isGhost.at(i) && isFilled.at(i)) {
            size_t emptySpot = probeEmptySpotForKey(buckets[i].first);
            isFilled.set(i, false);
            isGhost.set(i, true);
            noProbeNoRehashInsertKeyValueAtIndex(buckets[i], emptySpot);
        }
    }
}

template<typename Key, typename Mapped>
void PersistentHashMap<Key, Mapped>::insert(const ValueType& keyValue) {
    this->writeLock();
    this->insertKeyValue(keyValue);
    this->unlock();
}

// these templates are getting a little ridiculous
template<typename Key, typename Mapped>
typename PersistentHashMap<Key, Mapped>::Iterator 
PersistentHashMap<Key, Mapped>::find(const Key& key) {
    this->readLock();
    size_t index = probeForExistingKey(key);
    this->unlock();
    return Iterator(index, this);
}

// returns an iterator to the first filled bucket
template<typename Key, typename Mapped>
typename PersistentHashMap<Key, Mapped>::Iterator 
PersistentHashMap<Key, Mapped>::begin() {
    // this should always be good
    // TODO: remove in release mode
    this->readLock();
    // no need to unlock on these, they'll kill the program
    assert(this->header->capacity == isFilled.size());
    assert(this->header->capacity == isGhost.size());
    size_t firstIndex;
    for (firstIndex = 0; firstIndex != this->header->capacity; ++firstIndex) {
        if (this->isFilled.at(firstIndex) && !this->isGhost.at(firstIndex)) {
            this->unlock();
            return Iterator(firstIndex, this);
        }
    }
    // if there's nothing then just return one past the end
    this->unlock();
    return this->end();
}

// returns an iterator to one past the end
template<typename Key, typename Mapped>
typename PersistentHashMap<Key, Mapped>::Iterator 
PersistentHashMap<Key, Mapped>::end() {
    this->readLock();
    size_t rvCapacity = this->header->capacity;
    this->unlock();
    return Iterator(rvCapacity, this);
}

template<typename Key, typename Mapped>
bool PersistentHashMap<Key, Mapped>::erase(const Key& key) {
    // find specified element
    this->writeLock();
    auto indexForElement = this->probeForExistingKey(key);
    if (indexForElement == this->header->capacity) {
        this->unlock();
        return false;
    }
    // delete the element from bucket and decrement numElements
    this->isGhost.set(indexForElement, true);
    --this->header->numElements;
    this->unlock();
    return true;
}


template<typename Key, typename Mapped>
size_t PersistentHashMap<Key, Mapped>::capacity() {
    this->readLock();
    size_t rv = this->header->capacity;
    this->unlock();
    return rv;
}

template<typename Key, typename Mapped>
size_t PersistentHashMap<Key, Mapped>::size() {
    this->readLock();
    size_t rv = this->header->numElements;
    this->unlock();
    return rv;
}

template<typename Key, typename Mapped>
bool PersistentHashMap<Key, Mapped>::empty() {
    this->readLock();
    auto rv = this->header->elements == 0;
    this->unlock();
    return rv;
}

// Do we really need these?
template<typename Key, typename Mapped>
void PersistentHashMap<Key, Mapped>::readLock() {
    header->rwLock.readLock();
}

template<typename Key, typename Mapped>
void PersistentHashMap<Key, Mapped>::writeLock() {
    header->rwLock.writeLock();
}

template<typename Key, typename Mapped>
void PersistentHashMap<Key, Mapped>::unlock() {
    header->rwLock.unlock();
}

template<typename Key, typename Mapped>
void PersistentHashMap<Key, Mapped>::printState() {
    std::cout << "-----" << std::endl;
    for (size_t i = 0; i < this->header->capacity; ++i) {
        if (this->isGhost.at(i)) {
            fprintf(stdout, "%s", "[--]\n");
            continue;
        } else if (!this->isFilled.at(i)) {
            fprintf(stdout, "%s", "[]\n");
            continue;
        }
        // if an element is present then print the element
        std::cout << "[" << this->buckets[i].first << "] " << this->buckets[i].second << std::endl;
    }
    std::cout << "-----" << std::endl; 
}


#endif
