#include <iostream>
#include <cassert>
#include "http.hpp"
#include "thread_queue.hpp"

//--std=c++11 -I/Users/graham/grahameger_com/eecs/398/project/include /Users/graham/grahameger_com/eecs/398/project/src/http.cpp

int main(int argc, char *argv[]) {
	threading::ThreadQueue<int> q;
	q.push(1);
	q.push(2);
	int one = q.pop();
	int two = q.pop();
	assert(one == 1 && two == 2);
	search::HTTPClient client;
	client.SubmitURLSync("http://example.com/index.html");
	client.SubmitURLSync("http://neverssl.com/index.html");
	client.SubmitURLSync("https://grahameger.com");
	client.SubmitURLSync("https://stackoverflow.com/questions/27205810/how-recv-function-works-when-looping");
}