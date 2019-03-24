#include "hash_table.hpp"

	hash_table::hash_table(){}

  //might destruct in save()...
  hash_table::~hash_table(){
		if(pointers == true){
			for(int i=0; i<numBuckets; i++){
				for(hashStruct entry : array[i]){
					delete entry.offset;
				}
			}
		}
	}

	int hashtable::operator[](String key){
		int hash = hash(key);
		hash = hash%numBuckets;
		for(hashStruct entry : array[hash]){
			if(entry.key.compare(key)){
				//key already exists
				if(pointers == true){
					//regular hash table
					return *entry.offset;
				}
				else{
					//now an offset table
					return entry.offset;
				}
			}
		}
		//create a new entry
		String* offset = new String;
		hashStruct new_entry = {key, offset};
    array[hash].push_back(new_entry);
		entries++;
		return offset;
	}

	int hash_table::size(){
		return entries;
	}

	void hash_table::load(String indexFilename, String tableFilename){

	}

	void hash_table::save(char* indexFilename,char* tableFilename){
		pointers = false;
		hashStruct offsets[entries];
		ifstream indexFile(indexFilename);
		ifstream tableFile(tableFilename);
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

		tableFile<< objectsPerBucket << entries << offsets;
	}

	//murmur
	int hash_table::hash(String key){
		int len = key.size();
		const unsigned int m = 0x5bd1e995;
		const int r = 24;
		unsigned int h = 4 ^ len;
		const unsigned char* data = (const unsigned char *) key;
		while(len >= 4){
			unsigned int k = *(unsigned int*) data;
			k = k*m;
			k ^= k>>r;
			k = k*m;
			h = h*m;
			h = h^k;
			data = data + 4;
			len = len - 4;
		}
		switch(len){
			case 3: h ^= data[2] << 16;
			case 2: h ^= data[1] <<8;
			case 1: h ^= data[0];
							h *=m;
		}
		h ^= h>>13;
		h *= m;
		h ^= h>>15;

		return h;
	}


