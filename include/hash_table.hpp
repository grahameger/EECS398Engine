#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <cstring>
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

   struct hashStruct{
      String key;
      T* offset;
   };
	static const int numBuckets = 4096;
   std::list<hashStruct> array[numBuckets];
private:
	//my very own hash function
	long hash(String key);
	//number of keys in the table
   int entries = 0;
	//number of bytes a char* would be to hold the index, not this table
   int byteLength;
   String index;
   bool pointers = true;
};
 
  

	template <typename T>
	hash_table<T>::hash_table(){}

  //might destruct in save()...
	template <typename T>
  hash_table<T>::~hash_table(){
		if(pointers == true){
			for(int i=0; i<numBuckets; i++){
				for(auto it = array[i].begin(); it!=array[i].end(); it++){
					delete (*it).offset;
				}
			}
		}
	}

	template <typename T>
	T* hash_table<T>::operator[](String key){
		long hashed = hash(key);
		hashed = hashed%numBuckets;
		//std::cout<<key.CString()<<" "<<hashed<<std::endl;
		for(auto it = array[hashed].begin(); it!=array[hashed].end(); it++){
			if((*it).key.Compare(key)){
				//key already exists
				return (*it).offset;
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
		std::ifstream tableFile(tableFilename.CString());
		int objectsPerBucket;
		//first we read number of entries
		tableFile.seekg(0, tableFile.end);
		int length = tableFile.tellg();
		tableFile.seekg(0, tableFile.beg);
		char* buf = new char[length];
		int location = 0;
		tableFile.read(buf, length);
		std::memcpy(&entries, buf + location, sizeof(entries));
		location += sizeof(entries);
    pointers = false;
		hashStruct offset;
		T* holder;
	  char reader[30];
		for(int i = 0; i < numBuckets; i++){
			std::memcpy(&objectsPerBucket, buf + location, sizeof(objectsPerBucket));
			location += sizeof(objectsPerBucket);
			//free heap thats about to be written over
			for(auto it = array[i].GetFront(); it!=array[i].End(); it++){
				delete (*it).offset;
			}
			//array[i].clear();our list doesnt have clear
			for(int j = 0; j<objectsPerBucket; j++){
				//read in the key
				for(int k = 0; k < 30; k++){
					if(*(buf + location + k) != '\0'){
						reader[k] = *(buf + location + k);
					}
					else{
						reader[k] = '\0';
						location += k + 1;
						break;
					}
				}
				//need to copy char* into string, not ove it
				String constMagic((const char*) reader);
				offset.key = constMagic;
				std::memcpy(&holder, buf + location, sizeof(holder));
				location += sizeof(holder);
				offset.offset = holder;
				array[i].addToBack(offset);			

			}
		}
		delete[] buf;
		tableFile.close();

	}

	template <typename T>
	void hash_table<T>::saveIndex(String indexFilename,String tableFilename){
		std::ofstream indexFile(indexFilename.CString(), std::ofstream::trunc);
		std::ofstream tableFile(tableFilename.CString(), std::ofstream::trunc);
		tableFile.write(reinterpret_cast<const char*>(&entries), sizeof(entries));
		long location = 0;
		for(int i = 0; i < numBuckets; i++){
			int sizeHolder = 0;//array[i].size();out list doesnt have .size()
			tableFile.write(reinterpret_cast<const char*>(&sizeHolder), sizeof(sizeHolder));
			for(auto j = array[i].GetFront(); j!=array[i].End(); j++){
				tableFile << ((*j).key).CString();
			  tableFile << '\0';
				tableFile.write(reinterpret_cast<const char*>(&location), sizeof(location));
				//std::cout<< sizeof(location) <<std::endl;
				//get hashstruck, get its offset, dereference offset, get size
				String offset = *((*j).offset);
				location += offset.Size();
				indexFile << offset.CString();

			}
		}
		indexFile.close();
		tableFile.close();
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
