//  Created by Jake C on 2/10/19.
#ifndef Parser_hpp
#define Parser_hpp
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

class LinkFinder {
public:
    //constructor
    LinkFinder();
    
    //destructor
    ~LinkFinder();
    
    //returns -1 if something failed, else returns 0
    //parses html file into title, body, links, anchor
    int parse(char *filename);
    
private:
    //sets index pointer to 1 place after string
    //must know string can be found. will not work for malformed html
    void find_string(char *html_file, char* find, long *index, long file_length);
    //script tags
    bool is_script(char *html_file, long *index, long file_length);
    bool is_style(char *html_file, long *index, long file_length);
    bool is_title(char *html_file, long *index, long file_length);
    //prints word between > (here) < for title and anchor text
    void get_anchor_word(char *html_file, long *index, long file_length, char* word);
    
};
#endif /* Parser_hpp */
// -----------------------------------------------------------------------------
//TESTING + HOW TO USE
/*
 #include "Parser.h"
 #include <ctime>
 using namespace std;
 
 int main(int argc, const char * argv[]) {
 
 std::clock_t start;
 double duration = 0;
 start = std::clock();
 LinkFinder L;
 string word;
 L.parse((char*)argv[1]);
 duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
 std::cout<<"printf: "<< duration <<'\n';
 std::cout << "end";
 return 0;
 }*/