#pragma once
#ifndef LRUCACHE_H_398
#define LRUCACHE_H_398

#include <unordered_map>
#include <list>
using std::list;
using std::unordered_map;

template <class Key, class Value>
class LRUCache
{
public:
	LRUCache(int capacityIn, bool valueIsDynamicallyAllocated= false);
	Value get(Key &key);
	void put(Key &key, Value &val);
	void clear();

private:
	void ChangeToMostRecentlyUsed(Key &key);
	void EvictLRU();
	unordered_map<Key, Value> Cache;
	unordered_map<Key, typename list<Key>::iterator> LRUQueueKeyPositions;
	list<Key> LRUQueue;
	int Capacity;
	bool ValueIsDynamicallyAllocated;
};

//implementation
template <class Key, class Value>
void LRUCache<Key, Value>::clear()
{
	while(!Cache.empty())
	   EvictLRU();
}

template <class Key, class Value>
LRUCache<Key, Value>::LRUCache(int capacity, bool valueIsDynamicallyAllocated)
	: Capacity(capacity), ValueIsDynamicallyAllocated(valueIsDynamicallyAllocated) {}

template <class Key, class Value>
void LRUCache<Key, Value>::EvictLRU()
{
	Key *lruKey = &LRUQueue.front();

	if(ValueIsDynamicallyAllocated)
	{
		Value valToDelete = Cache[*lruKey];
		delete valToDelete;
	}

	LRUQueue.pop_front();
	LRUQueueKeyPositions.erase(*lruKey);
	Cache.erase(*lruKey);
}

template <class Key, class Value>
void LRUCache<Key, Value>::ChangeToMostRecentlyUsed(Key &key)
{
	if(LRUQueueKeyPositions.find(key) != LRUQueueKeyPositions.end())
	{
		typename list<Key>::iterator mruIt = LRUQueueKeyPositions[key];
		LRUQueue.erase(mruIt);
	}


	LRUQueue.push_back(key);
	typename list<Key>::iterator lruIt = LRUQueue.end();
	lruIt--;
	LRUQueueKeyPositions[key] = lruIt;
}

template <class Key, class Value>
Value LRUCache<Key, Value>::get(Key &key)
{	
	if(Cache.find(key) == Cache.end()) throw 1; //key not in cache
	
	ChangeToMostRecentlyUsed(key);
	return Cache[key];
}

template <class Key, class Value>
void LRUCache<Key, Value>::put(Key &key, Value &val)
{
	Cache[key] = val;
	ChangeToMostRecentlyUsed(key);
	if(Cache.size() > Capacity) EvictLRU();
}

#endif