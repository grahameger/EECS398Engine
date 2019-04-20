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

static String top_level_domains[] = {"com", "org", "mil", "int", "edu", "gov", "net"};
const size_t NUM_TOP_DOMAINS = 7;
const int MAX_DOMAIN_RANK = 745000;
const size_t DOMAIN_LENGTH = 3;

class Index_object{
public:
    String word;
    char type;//body=b/anchor=a/title=t
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
    unsigned int domain_rank = MAX_DOMAIN_RANK; //1 = top 1, 2 = top 2, 3 = top 3, etc...
    bool is_https = false;
    
    Doc_object() : Links(200), Words(5000), anchor_words(200), url(5) { }
    
};

class LinkFinder {
public:
    //constructor
    LinkFinder();
    
    LinkFinder(char *html_file_in, size_t filesize_in, String url_in, bool is_https_in) : html_file(html_file_in), file_length(filesize_in), url(url_in), is_https(is_https_in) {}
    
    
    //destructor
    ~LinkFinder();
    bool is_english;
    
    //returns -1 if something failed, else returns 0
    //parses html file into title, body, links, and anchor text
    int parse_html();
    
    //parses url
    void parse_url(std::vector<std::pair<std::string, int>> &v);
    
    bool is_https;
    String url;
    char* html_file;
    unsigned long file_length = 0;
    unsigned long index = 0;
    
    void print_all() {
        for(int i = 0; i < Document.Links.size(); i ++) {
            std::cout << Document.Links[i].CString() << std::endl;
        }
        std::cout << std::endl << std::endl;
        for(int i = 0; i < Document.Words.size(); i++) {
            std::cout << Document.Words[i].word.CString() << ":" << Document.Words[i].type << ":" << Document.Words[i].position << std::endl;
        }
    }
    
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
    
    //Finds word and what type they belong to
    void get_words(char *html_file, char type);
    
    //resets the file pointer to reset_value. Good to use after find_string.
    void reset_index(unsigned long reset_value);
    
    //Prints out all words from *index to stop_index. Ignores everything in <...>
    void get_anchor_text(char *html_file, unsigned long stop_index);
    
    //Finds <a> tag's parent tag if exists. Then finds position of either the
    //parent's closing tag or </a>, whichever occurs first. Then prints out anchor
    //text between <a>....position.
    void find_closing_a_tag(char *html_file);
    
    //returns min of index1 and index2
    long get_min(unsigned long index1, unsigned long index2);
    
    //returns max of index1 and index2
    long get_max(unsigned long index1, unsigned long index2);
    
    //sets file pointer to start of opening parent tag
    bool find_open_tag(char *html_file);
    
    //If parent tag exists, returns position of parent tag close.
    unsigned long parent_tag_distance(char *html_file, char* tag);
    
    //Adds character to word if it's a relevant char. Lowers it as well.
    void add_char_to_word(char *html_file, String &word, char type, Vector<Index_object> &v);
    
    //Assigns rank to document if word is found in alexa top list
    void assign_domain_rank(const String &word, std::vector<std::pair<std::string, int>> &v);
    
    //Checks if word in url is one of 7 stopwords
    bool is_stop_domain(String &word);
    
};

bool is_space(char c);
bool is_relevant_char(char c);
bool is_vowel(char c);
bool is_valid_word(String word, unsigned int vowels);

#endif /* Parser_hpp_398 */
