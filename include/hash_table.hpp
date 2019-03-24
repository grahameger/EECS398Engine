#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <fstream>
#include <list>
#include "String.h"


template <typename T>
class hash_table{
public:
	hash_table();
  ~hash_table();
	//returns a pointer to the string
	T* operator[](String key);
  //returns number of entries
	int size();
	//saves table and index to disk
  void load(String indexFilename, String tableFilename);

  void saveIndex(String indexFilename, String tableFilename);

private:
  struct hashStruct{
	  String key;
	  T* offset;
  };
	//my very own hash function
	long hash(String key);
	//number of keys in the table
  int entries = 0;
	//number of bytes a char* would be to hold the index, not this table
  int byteLength;
	static const int numBuckets = 4096;
  String index;
  bool pointers = true;
	std::list<hashStruct> array[numBuckets];
};
 
  

	template <typename T>
	hash_table<T>::hash_table(){}

  //might destruct in save()...
	template <typename T>
  hash_table<T>::~hash_table(){
		if(pointers == true){
			for(int i=0; i<numBuckets; i++){
				for(hashStruct entry : array[i]){
					delete entry.offset;
				}
			}
		}
	}

	template <typename T>
	T* hash_table<T>::operator[](String key){
		long hashed = hash(key);
		hashed = hashed%numBuckets;
		for(hashStruct entry : array[hashed]){
			if(entry.key.Compare(key)){
				//key already exists
				return entry.offset;
			}
		}
		//create a new entry
		T* offset = new T;
		hashStruct new_entry = {key, offset};
    array[hashed].push_back(new_entry);
		entries++;
		return offset;
	}

	template <typename T>
	int hash_table<T>::size(){
		return entries;
	}

	template <typename T>
	void hash_table<T>::load(String indexFilename, String tableFilename){
		std::ifstream indexFile(indexFilename);
		std::ifstream tableFile(tableFilename);
		int objectsPerBucket;
		tableFile >> entries;
    pointers = false;
		hashStruct offset;
		for(int i = 0; i < numBuckets; i++){
			tableFile >> objectsPerBucket;
			//free heap thats about to be written over
			for(auto it = array[i].begin(); it!=array[i].end(); it++){
				delete *it.offset;
			}
			array[i].clear();
			for(int i = 0; i<objectsPerBucket; i++){
				tableFile>>offset;
				std::cout<<offset<<std::endl;
			}
		}
		

	}

	template <typename T>
	void hash_table<T>::saveIndex(String indexFilename,String tableFilename){
		pointers = false;
		hashStruct offsets[entries];
		std::ofstream indexFile(indexFilename.CString());
		std::ofstream tableFile(tableFilename.CString());
		int objectsPerBucket[numBuckets];
		int location = 0;
		int counter = 0;
		for(int i = 0; i < numBuckets; i++){
			objectsPerBucket[i] = array[i].size();
			for(auto j = array[i].begin(); j!=array[i].end(); j++){
				offsets[counter].key = j.key;
				offsets[counter].offset = location;
				location += *(j.offset).size();
				indexFile << *(j.offset);

				counter++;
			}
		}

		tableFile<< entries << objectsPerBucket << offsets;
	}

	//murmur
	template <typename T>
	long hash_table<T>::hash(String key){
		long hash = 0;
		const char* k = key.CString();		
		while(*k != '\0'){
			hash = (hash << 4) + *(k++);
			long magic = hash & 0xF0000000L;
			if( magic != 0){
				long holder = (unsigned long) magic>>24;
				hash ^= holder;
			}
			hash &= ~magic;

		}
		return hash;
	}	

#endif
