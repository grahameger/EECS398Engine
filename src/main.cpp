#include <iostream>
#include <fstream>
#include <cassert>
#include "thread_queue.hpp"
#include "hash_table.hpp"
#include "String.h"
//--std=c++11 -I/Users/graham/grahameger_com/eecs/398/project/include /Users/graham/grahameger_com/eecs/398/project/src/http.cpp

int main(int argc, char *argv[]) {
  hash_table<String> h;
  String s = "abc";
	*(h[s]) = s;
  std::cout<<(*h[s]).CString()<<std::endl;


  *(h[s]) = "123";
  std::cout<<(*h[s]).CString()<<std::endl;

	String a = "am";
	*h[a] = "pola";
	std::cout<<"asdfasg"<<std::endl;
  std::cout<<(*h[a]).CString()<<std::endl;

	hash_table<int> h2;
	*(h2[a]) = 0;
	std::cout<<(*h2[a])<<std::endl;

	String in = "index.txt";
	String table = "table.txt";
	
	h.saveIndex(in, table);
  hash_table<String> h3;
	h3.load(in, table);

	std::ifstream i(in.CString());
	//usage
	char* input = new char[1000];
	i >> input;
	std::cout<<input<<std::endl;
	char* v = input + (long)h3["abc"];
	std::cout<<*v<<std::endl;
	delete[] input;
}
