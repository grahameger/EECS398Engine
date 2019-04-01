// Created by Graham Eger on 4/1/2019

#pragma once
#ifndef PERSISTENTHASHMAP_H_398
#define PERSISTENTHASHMAP_H_398

#include <cstdlib>
#include "Pair.h"
#include "String.h"

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
    SizeType numElements;
    double loadFactor;

    ValueType * buckets;
    static const SizeType INITIAL_CAPACITY = 1;

};

#endif