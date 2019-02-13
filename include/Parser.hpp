//  Created by Jake C on 2/10/19.
#ifndef Parser_hpp
#define Parser_hpp
#include <regex>
#include <iostream>
#include <stdio.h>

class LinkFinder {
public:
    //constructor
    LinkFinder();
    
    //destructor
    ~LinkFinder();
    
    //EFFECTS: Takes an HTML file as a char*, an empty array, and size ptr as inputs.
    //         Modifies result[] by adding found links. returns size as # of links.
    //         If no links found, Size is 0.
    //MODIFIES: result[], Size
    //*NOTE: Only finds links in 'a'/'A' tags where link is in "" or ''
    void findLinks(const char* html_file, char* result[], int* size);
    
private:
    char** linkArray;
    int numLinks = 0;
};
#endif /* Parser_hpp */
/* -----------------------------------------------------------------------------
TESTING + HOW TO USE
int main(int argc, const char * argv[]) {
 char file[1000] = "<!DOCTYPE html> \n<html lang=\"en\">\n<head>\n<title>insta485</title>\n</head>\n<body>\n<div style=\"position: absolute; top: 0; left: 0; width: 200px; text-align:left;\">\n<a href='/'>Timeline</a>\n</div>\n<div style=\"position: absolute; top: 0; right: 0; width: 200px; text-align:right;\">\n<a href=\"/explore/\">explore</a> <a href=\"/u/awdeorio/\">awdeorio</a>\n</div>\n<a href=\"/u/awdeorio/\">\n<img src=\"/uploads/e1a7c5c32973862ee15173b0259e3efdb6a391af.jpg\" alt=\"image\">\n</a>\n<h2><a href=\"/u/awdeorio/\">awdeorio</a> <a href=\'/p/3/\'>a minute ago</a></h2>\n";
 
 char* s = new char[5000];
 strcpy(s,file);
 char *linkArray[100]; assuming no more than 100 links/page
 int size = 0;
 LinkFinder L;
 L.findLinks(s, linkArray, &size);
 for(int i = 0; i < size; i++) {
 std::cout << linkArray[i] << std::endl;
 }
 std::cout << "end ";
 delete[] s;
 
 return 0;
 }
 */