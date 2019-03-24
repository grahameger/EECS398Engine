#include <iostream>
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
}
