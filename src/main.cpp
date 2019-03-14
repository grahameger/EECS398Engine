#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <mutex>
#include "ThreadPool.hpp"
#include "http.hpp"
#include "thread_queue.hpp"
#include "semaphore.hpp"

//--std=c++11 -I/Users/graham/grahameger_com/eecs/398/project/include /Users/graham/grahameger_com/eecs/398/project/src/http.cpp
const size_t MAX_THREADS = 10000; 

void wrapper(search::HTTPClient * client, const std::string url) {
	client->SubmitURLSync(url);	
}

// NEED to create an array of threads that's a fixed size
// use a free list of integers that indicates which ones
// are free to use. The stack is overflowing because
// we're creating all threse threads but they're not
// ending 

int main(int argc, char *argv[]) {
	threading::ThreadQueue<int> q;
	q.push(1);
	q.push(2);
	int one = q.pop();
	int two = q.pop();
	assert(one == 1 && two == 2);
	search::HTTPClient client;

	std::vector<std::string> urls;
	urls.reserve(1000000);
	std::ifstream start_list("../test/parsed.urls");
	std::string line;

	ThreadPool pool(MAX_THREADS);

	while (std::getline(start_list, line)) {
		pool.enqueue(wrapper, &client, line);
	}

	// client.SubmitURLSync("http://example.com/index.html");
	// client.SubmitURLSync("http://neverssl.com/index.html");
	// client.SubmitURLSync("https://grahameger.com");
	// client.SubmitURLSync("https://stackoverflow.com/questions/27205810/how-recv-function-works-when-looping");
}