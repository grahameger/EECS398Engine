#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include "http.hpp"
#include "thread_queue.hpp"
#include "semaphore.hpp"

//--std=c++11 -I/Users/graham/grahameger_com/eecs/398/project/include /Users/graham/grahameger_com/eecs/398/project/src/http.cpp
const size_t MAX_THREADS = 1000; 
threading::Semaphore sem(MAX_THREADS);

void * wrapper(search::HTTPClient * client, threading::Semaphore * sem, const std::string url) {
	client->SubmitURLSync(url);	
	sem->notify();
	return nullptr;
}

void do_join(std::thread& t) {
	t.join();
}

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

	std::vector<std::thread> v;
	while (std::getline(start_list, line)) {
		sem.wait();
		v.push_back(std::thread(wrapper, &client, &sem, line));
	}

	std::for_each(v.begin(), v.end(), do_join);

	// client.SubmitURLSync("http://example.com/index.html");
	// client.SubmitURLSync("http://neverssl.com/index.html");
	// client.SubmitURLSync("https://grahameger.com");
	// client.SubmitURLSync("https://stackoverflow.com/questions/27205810/how-recv-function-works-when-looping");
}