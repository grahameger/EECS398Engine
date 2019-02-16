#include "Parser.hpp"
using std::regex;

//default constructor
LinkFinder::LinkFinder() {}

//destructor
LinkFinder::~LinkFinder() {
    for (int i = 0; i < numLinks; i++)
    {
        delete [] linkArray[i];
    }
    delete [] linkArray;
}

void LinkFinder::findLinks(const char* html_file, char* result[], int* size) {
    int start_positions [1000];//assuming at most 100 links per page
    int end_positions [1000];
    int start_index = 0;
    int end_index = 0;
    bool a_tag = false;
    //Iterate through file. Get start and end positions of links
    for(int i = 0; i < strlen(html_file); i++) {
        //Found an <a or <A tag
        if(i+2 < strlen(html_file) && html_file[i] == '<' && (html_file[i+1] == 'a' || html_file[i+1] == 'A') && html_file[i+2] == ' ') {
            start_positions[start_index] = i;
            a_tag = true;
            start_index++;
            i += 2;
        }
        if(a_tag) {
            //Found closing tag
            if(html_file[i] == '>') {
                end_positions[end_index] = i + 1;
                end_index++;
                a_tag = false;
            }
        }
    }
    //copy links into linkArray using start/end positions
    numLinks = end_index;
    *size = end_index;
    linkArray = new char *[numLinks];
    for(int i = 0; i < numLinks; i++) {
        int length = end_positions[i] - start_positions[i] + 1;
        char *substring = new char[length];
        memcpy(substring, &html_file[start_positions[i]], end_positions[i] - start_positions[i]);
        substring[end_positions[i] - start_positions[i]] = '\0';
        linkArray[i] = substring;
    }
    //use regex on each substring to find link
    //**NOTE:Finds links only if in "" or ''
    regex regx("(?:href=[\"']([^ >\"'\n]+))|(?:href=([^ >\"'\n]+))", std::regex_constants::icase);
    std::cmatch m;
    for(int i = 0; i < numLinks; i++) {
        if(std::regex_search(linkArray[i], m, regx) ) {
            if(!m[1].matched) {
                strcpy(linkArray[i], m[2].str().c_str());
            }
            else {
                strcpy(linkArray[i], m[1].str().c_str());
            }
            result[i] = linkArray[i];
        }
    }
    return;
}