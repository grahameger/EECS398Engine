#include <string>
#include <iostream>
#include "RobotsTxt.h"

using namespace std;

string UsageString = "Usage: ./RobotsSaddle robots.txt [robots2.txt ... robotsN.txt]";

int main(int argc, char** argv) {
	if(argc < 2) {
		cout << UsageString << endl;
		return 0;
	}

	for(int i = 1; i < argc; i++) {
		RobotsTxt robots( argv[i] );
	}
}
