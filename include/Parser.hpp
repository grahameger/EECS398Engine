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
// -----------------------------------------------------------------------------
//TESTING + HOW TO USE
/*#include "Parser.h"
 #include <sys/types.h>
 #include <sys/mman.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 using namespace std;
 Filename is argument 1
 int main(int argc, const char * argv[]) {
 int fd = open(argv[1], O_RDONLY);
 long long len = lseek(fd, 0, SEEK_END);
 void *data = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
 char *linkArray[1000];
 int size = 0;
 LinkFinder L;
 L.findLinks((char*)data, linkArray, &size);
 for(int i = 0; i < size; i++) {
 std::cout << linkArray[i] << std::endl;
 }
 std::cout << "end ";
 //delete[] s;
 
 return 0;
 }
 */