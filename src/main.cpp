#include <iostream>
#include <cassert>
#include "http.hpp"
#include "thread_queue.hpp"

//--std=c++11 -I/Users/graham/grahameger_com/eecs/398/project/include /Users/graham/grahameger_com/eecs/398/project/src/http.cpp

int main(int argc, char *argv[]) {
	search::ThreadQueue<int> q;
	q.push(1);
	q.push(2);
	int one;
	int two;
	while (!q.pop(one)) {}
	while (!q.pop(two)) {}
	assert(one == 1 && two == 2);
	search::HTTPClient client;
	client.SubmitURL("http://example.com/index.html");
}