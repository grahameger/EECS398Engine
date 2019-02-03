#include <iostream>
#include "http.hpp"

//--std=c++11 -I/Users/graham/grahameger_com/eecs/398/project/include /Users/graham/grahameger_com/eecs/398/project/src/http.cpp

using namespace std;
int main(int argc, char *argv[]) {
	search::HTTPClient client;
	client.SubmitURL("http://example.com/index.html");
}