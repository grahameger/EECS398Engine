//  Created by Jake C on 2/10/19.
#ifndef Parser_h
#define Parser_h
#include <regex>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
using std::regex;

class LinkFinder {
public:
    //constructor
    LinkFinder();
    
    //destructor
    ~LinkFinder();
    
    //EFFECTS: Atomically rewrites filename with stripped html
    //         Returns -1 on failed file creation, 0 on success
    //MODIFIES: HTML file
    int stripTags(const char* filename);
    
    //EFFECTS: Takes an HTML filename and creates a new file called links<filename>
    //         Where each link is separated by a space. If no links found, file is empty.
    //         Returns -1 on failed file creation, 0 on success
    //MODIFIES: result[], Size
    int findLinks(const char* filename);
    
private:
};
#endif /* Parser_h */
// -----------------------------------------------------------------------------
//TESTING + HOW TO USE
/*
 #include "Parser.h"
 #include <ctime>
 #include <fstream>
 using namespace std;
 
 int main(int argc, const char * argv[]) {
 std::clock_t start;
 double duration = 0;
 start = std::clock();
 LinkFinder L;
 L.findLinks(argv[1]);
 string word;
 int count = 0;
 ifstream ifs("linksfile3.txt");
 if(!ifs.is_open()) {
 cout << "Error opening linksfile3.txt" << endl;
 exit(EXIT_FAILURE);
 }
 while(ifs >> word) {
 cout << word << endl;
 count++;
 }
 cout << count << endl;
 L.stripTags(argv[1]); **ALWAYS CALL STRIPTAGS AFTER AS IT LITERALLY CHANGES FILE
 duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
 std::cout<<"printf: "<< duration <<'\n';
 std::cout << "end";
 return 0;
 }*/