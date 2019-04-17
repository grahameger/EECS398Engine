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

struct Doc_object {
    String doc_url;
    unsigned short num_slash_in_url = 0;
    Vector<String> Links;
    Vector<Index_object> Words;
    Vector<Vector<Index_object>> anchor_words;
    Vector<String> url;
    bool is_top_domain = false; //if in graham's list of top *100* domains.
    char domain_type = 'x'; // default x. com = c, mil = m, edu = e, none = x, gov = g, etc...
    unsigned long domain_rank; //1 = top 1, 2 = top 2, 3 = top 3, etc...
    bool is_https = false;
};

class LinkFinder {
public:
    //constructor
    LinkFinder();
    
    //destructor
    ~LinkFinder();
    bool is_english = true;
    
    //returns -1 if something failed, else returns 0
    //parses html file into title, body, links, and anchor text
    int parse(char *filename, size_t filesize, String url, bool is_https);//String url
    
    unsigned long file_length = 0;
    unsigned long index = 0;
    
    void print_all() {
        for(int i = 0; i < Document.Links.size(); i ++) {
            std::cout << Document.Links[i].CString() << std::endl;
        }
        std::cout << std::endl << std::endl;
        for(int i = 0; i < Document.Words.size(); i++) {
            std::cout << Document.Words[i].word.CString() << ":" << Document.Words[i].type.CString() << ":" << Document.Words[i].position << std::endl;
        }
    }
    
    //parses url
    void url_parser(String url);
    
    Doc_object Document;
    
private:
    //sets index pointer to 1 place after string
    //must know string can be found.
    //will skip entire html file if no </a> on the page. else skips to next </a>
    //be careful to reset index pointer to ensure still in range
    void find_string(char *html_file, char* find_lower, char* find_upper);
    
    
    //Look for href= If doesn't exist in <a>, returns false. Else, true.
    bool find_link(char *html_file, char* find_lower, char* find_upper);
    
    //If tag found, returns true. Else, false;
    bool is_script(char *html_file);
    bool is_style(char *html_file);
    bool is_title(char *html_file);
    bool is_html(char *html_file);
    
    //Prints out words line by line and which tag(word) they belong to
    void get_words(char *html_file, String word);
    
    //resets the file pointer to reset_value. Good to use after find_string.
    void reset_index(long reset_value);
    
    //Prints out all words from *index to stop_index. Ignores everything in <...>
    void get_anchor_text(char *html_file, long stop_index);
    
    //Finds <a> tag's parent tag if exists. Then finds position of either the
    //parent's closing tag or </a>, whichever occurs first. Then prints out anchor
    //text between <a>....position.
    void find_closing_a_tag(char *html_file);
    
    //returns min of index1 and index2
    long get_min(long index1, long index2);
    
    //returns max of index1 and index2
    long get_max(long index1, long index2);
    
    //sets file pointer to start of opening parent tag
    bool find_open_tag(char *html_file);
    
    //If parent tag exists, returns position of parent tag close.
    long parent_tag_distance(char *html_file, char* tag);
    
    //Adds character to word if it's a relevant char. Lowers it as well.
    void add_char_to_word(char *html_file, String &word, String type, Vector<Index_object> &v);
    
};

bool is_space(char c);
bool is_relevant_char(char c);
bool is_vowel(char c);
bool is_valid_word(String word, unsigned int vowels);

#endif /* Parser_hpp_398 */
