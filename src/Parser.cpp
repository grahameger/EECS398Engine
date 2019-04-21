
#include "Parser.hpp"
#include <cstring>


//default constructor
LinkFinder::LinkFinder() {}

//destructor
LinkFinder::~LinkFinder() {}

bool comp_pair (const std::pair<std::string,int> &t1, const std::pair<std::string,int> &t2) {
    return t1.first < t2.first;
}

Index_object & Index_object::operator=(const Index_object& rhs) {
    if(this == &rhs) {
        return *this;
    }
    word = rhs.word;
    type = rhs.type;
    position = rhs.position;
    return *this;
}

void LinkFinder::parse_url(Vector<std::pair<std::string, int>> &v) {
    unsigned int num_consonants_in_row = 0;
    unsigned int num_vowels = 0;
    String word;
    bool found_domain = false;
    for(size_t i = 0; i < url.Size(); i++) {
        //character is alpha
        if(isalpha(url.CString()[i])) {
            if(is_vowel(url.CString()[i])) {
                num_vowels++;
                num_consonants_in_row = 0;
            }
            else {
                num_consonants_in_row++;
            }
            word += tolower(url.CString()[i]);
            //reset if 4 chars in a row
            if(num_consonants_in_row == 4) {
                while(i < url.Size() && url.CString()[i] != '-' && url.CString()[i] != '/' && url.CString()[i] != '.' && url.CString()[i] != '_') {
                    i++;
                }
                word = "";//reset word
                num_consonants_in_row = 0;
                num_vowels = 0;
            }
        }
        else {
            if(is_valid_word(word, num_vowels)) {
                if(!found_domain && is_stop_domain(word)) {
                    found_domain = true;
                    for(size_t i = 0; i < Document.url.size(); i++) {
                        //check if in alexa list, assign rank
                        assign_domain_rank(Document.url[i], v);
                    }
                }
                Document.url.push_back(word);
            }
            if(url.CString()[i] == '/') {
                Document.num_slash_in_url++;
            }
            num_consonants_in_row = 0;//reset all
            num_vowels = 0;
            word = ""; //reset word
            while(i < url.Size() && url.CString()[i] != '-' && url.CString()[i] != '/' && url.CString()[i] != '.' && url.CString()[i] != '_') {
                if(url.CString()[i] == '/') {
                    Document.num_slash_in_url++;
                }
                i++;
            }
        }
    }
    if(is_valid_word(word, num_vowels)) {
        Document.url.push_back(word);
    }
    for(int i = 0; i < Document.url.size(); i++) {
        std::cout << Document.url[i].CString() << std::endl;
    }
    std::cout << Document.domain_rank << std::endl;
}

void LinkFinder::assign_domain_rank(const String &word, Vector<std::pair<std::string, int>> &v) {
    unsigned int rank = binSearch(v, 0, int(v.size()-1), word, MAX_DOMAIN_RANK);
    if(Document.domain_rank > rank) {
        Document.domain_rank = rank;
    }
}


bool LinkFinder::is_stop_domain(String &word) {
    if(Document.url.size() == 0) {
        return false;
    }
    if(word.Size() == 2) {
        Document.domain_type = 'u';
        return true;
    }
    for(size_t i = 0; i < NUM_TOP_DOMAINS; i++) {
        if(strncmp(word.CString(), top_level_domains[i].CString(), DOMAIN_LENGTH) == 0) {
            switch(i) {
                case 0 :
                    Document.domain_type = 'c';
                    break;
                case 1 :
                    Document.domain_type = 'o';
                    break;
                case 2 :
                    Document.domain_type = 'm';
                    break;
                case 3 :
                    Document.domain_type = 'i';
                    break;
                case 4 :
                    Document.domain_type = 'e';
                    break;
                case 5 :
                    Document.domain_type = 'g';
                    break;
                case 6 :
                    Document.domain_type = 'n';
                    break;
                default:
                    break;
            }
            return true;
        }
    }
    return false;
}

//rules
    /*
     1. 4 cons in a row
     2. No vowels
     3. Isalpha
     4. >= 2 chars
     **5. not stopword
     aqs, sourceid, chrome, utf, safari, firefox, com, google, search
     */

bool is_vowel(char c) {
    switch(c) {
        case 'a'  :
        case 'e'  :
        case 'i'  :
        case 'o'  :
        case 'u'  :
        case 'y'  :
            return true;
        default:
            return false;
    }
}

bool is_valid_word(String word, unsigned int vowels) {
    if(word.Size() >= 2 && vowels > 0 && strncmp(word.CString(), "www", 3) != 0) {
        return true;
    }
    return false;
}

int LinkFinder::parse_html() {
    Document.doc_url = url;
    Document.is_https = is_https;
    //url_parser(url, );
    while(index < file_length) {//run until end of file
        if(html_file[index] == '<') {
            is_link = false;
            (index)++;
            switch(html_file[index]) {
                case 'A'  :
                case 'a'  : //Link
                    if(html_file[index+1] == ' ') {
                        char find_up[] = "HREF=";
                        char find_low[] = "href=";
                        unsigned long reset_value = index;
                        if(find_link(html_file, find_low, find_up)) {
                            String link;
                            while(html_file[index] != ' ' && html_file[index] != '>') {
                                if(html_file[index] != '"' && html_file[index] != '\'') {
                                    is_link = true;
                                    link += html_file[index];
                                }
                                index++;
                            }
                            if(is_link) {
                                link_and_anchor object;
                                object.link_url = link;
                                Document.vector_of_link_anchor.push_back(object);
                            }
                        }
                        
                        //reset index in case link not found
                        reset_index(reset_value);
                        
                        //close the a tag
                        while(index < file_length && html_file[index] != '>') {
                            index++;
                        }
                        //set ptr to before >
                        index--;
                        //find a tag close. Parent or </a>
                        find_closing_a_tag(html_file);
                        while(index < file_length && html_file[index] != '>') {
                            index++;
                        }
                        index++;
                    }
                    //not a link, was <a(some char), goto default since regular tag
                    else {
                        goto DEFAULT;
                    }
                    break;
                   
                case 'S'  :
                case 's'  : //script/style Want to completely skip these
                    if(is_style(html_file)) {
                        char find_up[] = "</STYLE>";
                        char find_low[] = "</style>";
                        find_string(html_file, find_low, find_up);
                    }
                    else if(is_script(html_file)) {
                        char find_up[] = "</SCRIPT>";
                        char find_low[] = "</script>";
                        find_string(html_file, find_low, find_up);
                    }
                    else { //was not script or style tag, treat as ordinary tag
                        goto DEFAULT;
                    }
                    break;
                    
                case 'T'  :
                case 't'  :
                    //found <t, if <title, get it
                    if(is_title(html_file)) {
                        char type = 't';
                        get_words(html_file, type);
                    }
                    else {//was not <title, treat as ordinary tag
                        goto DEFAULT;
                    }
                    break;
                case 'H'  :
                case 'h'  :
                    if(is_html(html_file)) {
                        char find_low[] = "lang=";
                        char find_up[] = "LANG=";
                        unsigned long reset_value = index;
                        if(find_link(html_file, find_low, find_up)) {
                            if(index + 3 < file_length) {
                                if((html_file[index] == '"' || html_file[index] == '\'') && (html_file[index+1] == 'e' && html_file[index+2] == 'n')) {
                                    reset_index(reset_value);
                                    goto DEFAULT;
                                }
                                else if(html_file[index] == 'e' && html_file[index+1] == 'n') {
                                    reset_index(reset_value);
                                    goto DEFAULT;
                                }
                                else {
                                    is_english = false;
                                    return 0;
                                }
                            }
                        }
                    }
                    else {
                        goto DEFAULT;
                    }
                    
                DEFAULT:default : //ordinary tag. Just skip everything between <...>
                    char find[] = ">";
                    find_string(html_file, find, find);
            }
        }
        //grab the body text
        else {
            char type = 'b';
            get_words(html_file, type);
        }
    }
    return 0;
}

bool LinkFinder::is_html(char *html_file) {
    if(index + 3 < file_length) {
        if(html_file[index] == 'h' && html_file[index+1] == 't' && html_file[index+2] == 'm' && html_file[index+3] == 'l') {
            index += 3;
            return true;
        }
        else if(html_file[index] == 'H' && html_file[index+1] == 'T' && html_file[index+2] == 'M' && html_file[index+3] == 'L') {
            index += 3;
            return true;
        }
    }
    return false;
}

bool LinkFinder::is_script(char *html_file) {
    if(index + 5 < file_length) {
        if(html_file[index] == 's' && html_file[index+1] == 'c' && html_file[index+2] == 'r' && html_file[index+3] == 'i' && html_file[index+4] == 'p' && html_file[index+5] == 't') {
            index += 5;
            return true;
        }
        else if(html_file[index] == 'S' && html_file[index+1] == 'C' && html_file[index+2] == 'R' && html_file[index+3] == 'I' && html_file[index+4] == 'P' && html_file[index+5] == 'T') {
            index += 5;
            return true;
        }
    }
    return false;
}

bool LinkFinder::is_style(char *html_file) {
    if(index + 4 < file_length) {
        if(html_file[index] == 's' && html_file[index+1] == 't' && html_file[index+2] == 'y' && html_file[index+3] == 'l' && html_file[index+4] == 'e') {
            index += 4;
            return true;
        }
        else if(html_file[index] == 'S' && html_file[index+1] == 'T' && html_file[index+2] == 'Y' && html_file[index+3] == 'L' && html_file[index+4] == 'E') {
                index += 4;
                return true;
            }
    }
    return false;
}

bool LinkFinder::is_title(char *html_file) {
    if(index + 4 < file_length) {
        if(html_file[index] == 't' && html_file[index+1] == 'i' && html_file[index+2] == 't' && html_file[index+3] == 'l' && html_file[index+4] == 'e') {
            index += 6;
            return true;
        }
        else if(html_file[index] == 'T' && html_file[index+1] == 'I' && html_file[index+2] == 'T' && html_file[index+3] == 'L' && html_file[index+4] == 'E') {
            index += 6;
            return true;
        }
    }
    return false;
}

void LinkFinder::find_string(char *html_file, char* find_lower, char* find_upper) {
    int string_length = (int)strlen(find_lower);
    int counter = 0;
    while(index < file_length && counter < string_length) {
        if(html_file[index] == find_lower[counter]  || html_file[index] == find_upper[counter]) {
            counter++;
        }
        else {
            counter = 0;
        }
        index++;
    }
        return;
}

bool LinkFinder::find_link(char *html_file, char* find_lower, char* find_upper) {
    int string_length = (int)strlen(find_lower);
    int counter = 0;
    while(index < file_length && counter < string_length && html_file[index] != '>') {
        if(html_file[index] == find_lower[counter]  || html_file[index] == find_upper[counter]) {
            counter++;
        }
        else {
            counter = 0;
        }
        index++;
    }
    if(counter == string_length) {
        return true;
    }
    return false;
}

void LinkFinder::get_words(char *html_file, char type) {
    while(index < file_length && html_file[index] != '<') {
        String word;
        add_char_to_word(html_file, word, type);
    }
}

bool is_space(char c) {
    if(c == '\n' || c== '\t' || c == '\r' || c == ' ') {
        return true;
    }
    return false;
}

bool is_relevant_char(char c) {
    if(c == '#' || c== '@' || c == '*' || c == '$' || c == '&' || c == '|' || c == '"' || c == '(' || c == ')' || c == ',' || c == '.' || c == '[' || c == ']' || c == '/' || c == '!' || c == '>' || c == '<' || c == ':' || c == ';' || c == '_' || c == '=' || c == '?' || c == '}' || c == '{' || c == '\'') {
        return false;
    }
    return true;
}

void LinkFinder::add_char_to_word(char* html_file, String &word, char type) {
    if(index < file_length && !is_space(html_file[index]) && is_relevant_char(html_file[index]) && html_file[index] != '-') {
        while(index < file_length && !is_space(html_file[index]) && is_relevant_char(html_file[index])) {
            if(isalpha(html_file[index])) {
                word += tolower(html_file[index]);
            }
            else if(!isascii(html_file[index])) {
                while(index < file_length && is_relevant_char(html_file[index]) && !is_space(html_file[index])) {
                    index++;
                }
                index--;
                goto skipword;
            }
            else {
                word += html_file[index];
            }
            index++;
        }
        Index_object new_obj;
        new_obj.word = word;
        new_obj.type = type;//this is type
        new_obj.position = (int)Document.Words.size();
        Document.Words.push_back(new_obj);
        if(type == 'a' && is_link) {
            Document.vector_of_link_anchor[Document.vector_of_link_anchor.size()-1].anchor_words.push_back(new_obj);
        }
    }
    else {
    skipword:
        index++;
    }
}

void LinkFinder::get_anchor_text(char *html_file, unsigned long stop_index) {
    char type = 'a';
    bool is_there_anchor = false;
    //Skip over all inner tags until we hit a's closing tag
    while(index < file_length && index < stop_index) {
        is_there_anchor = true;
        if(html_file[index] == '<') {
            while(index < file_length && html_file[index] != '>') {
                index++;
            }
            index++;
        }
        get_words(html_file, type);
    }
    index = stop_index;
    return;
}

void LinkFinder::reset_index(unsigned long reset_value) {
    index = reset_value;
    return;
}

long LinkFinder::get_min(unsigned long index1, unsigned long index2) {
    return (index1 < index2) ? index1 : index2;
}

long LinkFinder::get_max(unsigned long index1, unsigned long index2) {
    return (index1 > index2) ? index1 : index2;
}

void LinkFinder::find_closing_a_tag(char *html_file) {
    unsigned long reset_value = index + 2;
    unsigned long parent_end_position = 0;
    
    //find first open tag
    if(find_open_tag(html_file)) {
        //found parent opening <
        //get length, then name
        int length = 2;
        while(index < file_length && html_file[index] != '>') {
            length++;
            index++;
        }
        reset_index(index - length + 2);
        char parent_tag[length];
        int i = 0;
        for(; i < length - 1; i++) {
            parent_tag[i] = html_file[index];
            index++;
        }
        parent_tag[i] = '\0';
        //find parent closing tag position
        parent_end_position = parent_tag_distance(html_file, parent_tag);
    }
    else {
        parent_end_position = 0;//ensures </a> is used
    }
    
    //reset file pointer
    reset_index(reset_value);
    
    //find closing a tag position
    char find_lowa[] = "</a>";
    char find_upA[] = "</A>";
    find_string(html_file, find_lowa, find_upA);
    
    //actual tag close will be min distance of parent and a tag
    unsigned long stop_index = get_min(parent_end_position, index - 4);
    
    //in case of no parent, don't infinite loop
    if (stop_index <= reset_value) {
        stop_index = get_max(parent_end_position, index - 4);
    }
    
    //reset file pointer
    reset_index(reset_value);
    
    //Get the anchor text
    get_anchor_text(html_file, stop_index);
    return;
}


unsigned long LinkFinder::parent_tag_distance(char *html_file, char* tag) {
    int string_length = (int)strlen(tag);
    char end_tag[string_length + 2];
    end_tag[0] = '<';
    end_tag[1] = '/';
    int j = 2;
    for(int i = 1; i < string_length; i++) {
        end_tag[j] = tag[i];
        j++;
    }
    end_tag[j] = '\0'; //now = </parent>
    bool nested_tag = false;
    int counter = 0; //don't think theres a tag that will mess this up <gggggg>
    int end_tag_counter = 0;
    
    //Go through file and try to find parent's closing tag.
    //Don't get tricked by nested tags
    while(index < file_length) {
        if(html_file[index] == end_tag[end_tag_counter]) {
            end_tag_counter++;
            if(end_tag_counter == string_length + 1) {
                if(!nested_tag) {
                    index -= (strlen(end_tag) - 1);
                    return index;
                }
                else {
                    end_tag_counter = 0;
                    nested_tag = false;
                }
            }
        }
        else {
            end_tag_counter = 0;
        }
        if(html_file[index] == tag[counter]) {
            counter++;
            if(counter == string_length) {
                nested_tag = true;
                counter = 0;
            }
        }
        else {
            counter = 0;
        }
        index++;
    }
    return index;
}

bool LinkFinder::find_open_tag(char *html_file) {
    bool is_closing_tag = false;
    bool tag_found = false;
    int num_closing = 0;
    int num_opening = 0;
    while(index > 0) {
        while(index > 0 && index < file_length && html_file[index] != '>') { //find a tag
            if(html_file[index] == '>') {
                tag_found = true;
            }
        index--;
        }
        if(!tag_found) { //no parent tag found
            return false;
        }
        while(index > 0 && html_file[index] != '<') { //tag was found
            if(html_file[index] == '/') {
                num_closing++; //uneven tag
                is_closing_tag = true; //is closing tag
            }
            index--;
        }
        if(!is_closing_tag) {
            num_opening++; //Got here, then must be opening tag
        }
        is_closing_tag = false; //reset
        if(num_opening >= num_closing) { //check if tags are even
            return true;
        }
        index++;
    }
    return false;//never get here
}

unsigned int binSearch(Vector<std::pair<std::string, int>> v, int l, int r, String val, unsigned int max)
{
    while (l <= r) {
        int m = l + (r - l) / 2;
    
        if (v[m].first == val.CString()) {
            return v[m].second;
        }
        
        if (v[m].first < val.CString()) {
            l = m + 1;
        }
        else {
            r = m - 1;
        }
    }

    return max;
}
