//  Created by Jake C on 2/10/19.
//  Graham Eger added a cstdlib include on 4/1 to make compile on crawler machine
#pragma once
#ifndef Parser_hpp_398
#define Parser_hpp_398
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "vector.h"
#include <ctype.h>
#include "String.h"

class Index_object{
public:
    String word;
    String type;//body/anchor/title
    int position;//word 0,1,2,3 in in document
    //Assignment operator
    Index_object &operator=(const Index_object& rhs);
    
};

class LinkFinder {
public:
    //constructor
    LinkFinder();
    
    //destructor
    ~LinkFinder();
    
    //returns -1 if something failed, else returns 0
    //parses html file into title, body, links, and anchor text
    int parse(char * htmlFile, size_t fileSize);
    
    int word_count = 0;
    
    Vector<Index_object> Document_meta_data_list;
    
    Vector<String> Link_vector;
    
    void print_meta_objects() {
        for(size_t i = 0; i < Document_meta_data_list.size(); i++) {
            std::cout << Document_meta_data_list[i].position << " : " << Document_meta_data_list[i].word.CString() << " : " << Document_meta_data_list[i].type.CString() << std::endl;
        }
    }
    
    void print_links() {
        for(size_t i = 0; i < Link_vector.size(); i++) {
            std::cout << Link_vector[i].CString() << std::endl;
        }
    }
    
private:
    //sets index pointer to 1 place after string
    //must know string can be found.
    //will skip entire html file if no </a> on the page. else skips to next </a>
    //be careful to reset index pointer to ensure still in range
    void find_string(char *html_file, char* find_lower, char* find_upper, long *index, long file_length);
    
    //Look for href= If doesn't exist in <a>, returns false. Else, true.
    bool find_link(char *html_file, char* find_lower, char* find_upper, long *index, long file_length);
    
    //If tag found, returns true. Else, false;
    bool is_script(char *html_file, long *index, long file_length);
    bool is_style(char *html_file, long *index, long file_length);
    bool is_title(char *html_file, long *index, long file_length);
    
    //Prints out words line by line and which tag(word) they belong to
    void get_words(char *html_file, long *index, long file_length, String word);
    
    //resets the file pointer to reset_value. Good to use after find_string.
    void reset_index(long *index, long reset_value);
    
    //Prints out all words from *index to stop_index. Ignores everything in <...>
    void get_anchor_text(char *html_file, long *index, long file_length, long stop_index);
    
    //Finds <a> tag's parent tag if exists. Then finds position of either the
    //parent's closing tag or </a>, whichever occurs first. Then prints out anchor
    //text between <a>....position.
    void find_closing_a_tag(char *html_file, long *index, long file_length);
    
    //returns min of index1 and index2
    long get_min(long index1, long index2);
    
    //returns max of index1 and index2
    long get_max(long index1, long index2);
    
    //sets file pointer to start of opening parent tag
    bool find_open_tag(char *html_file, long *index, long file_length);
    
    //If parent tag exists, returns position of parent tag close.
    long parent_tag_distance(char *html_file, char* tag, long *index, long file_length);
};

#endif /* Parser_hpp_398 */