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

	RobotsTxt robots = RobotsTxt();

	for(int i = 1; i < argc; i++) {
		string domain = "";
		domain += ('0' + i);
		std::cout << domain << std::endl;
		std::string fileStr(argv[ i ]);
		robots.SubmitRobotsTxt( domain, fileStr );
	}

	while(true) {
		string domain, path;
		cin >> domain;

		if ( domain == "0" ) break;
		cin >> path;

		cout << (robots.GetRule( path, domain ) ? "allowed\n" : "disallowed\n");
	}
}
