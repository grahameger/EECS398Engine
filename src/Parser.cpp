#include "Parser.hpp"

//default constructor
LinkFinder::LinkFinder() {}

//destructor
LinkFinder::~LinkFinder() {}

Index_object & Index_object::operator=(const Index_object& rhs) {
    if(this == &rhs) {
        return *this;
    }
    word = rhs.word;
    type = rhs.type;
    position = rhs.position;
    return *this;
}


int LinkFinder::parse(char* html_file, String url) {
    Document.doc_url = url;
    long file_length = strlen(html_file);
    long num = 0;
    long *index = &num;
    while(*index < file_length) {//run until end of file
        if(html_file[*index] == '<') {
            (*index)++;
            
            switch(html_file[*index]) {
                case 'A'  :
                case 'a'  : //Link
                    if(html_file[*index+1] == ' ') {
                        char find_up[] = "HREF=";
                        char find_low[] = "href=";
                        long reset_value = *index;
                        if(find_link(html_file, find_low, find_up, index, file_length)) {
                            String link;
                            while(html_file[*index] != ' ' && html_file[*index] != '>') {
                                if(html_file[*index] != '"' && html_file[*index] != '\'') {
                                    link += html_file[*index];
                                }
                                (*index)++;
                            }
                            Document.Links.push_back(link);
                        }
                        
                        //reset index in case link not found
                        reset_index(index, reset_value);
                        
                        //close the a tag
                        while(*index < file_length && html_file[*index] != '>') {
                            (*index)++;
                        }
                        //set ptr to before >
                        (*index)--;
                        //find a tag close. Parent or </a>
                        find_closing_a_tag(html_file, index, file_length);
                        while(*index < file_length && html_file[*index] != '>') {
                            (*index)++;
                        }
                        (*index)++;
                    }
                    //not a link, was <a(some char), goto default since regular tag
                    else {
                        goto DEFAULT;
                    }
                    break;
                    
                case 'S'  :
                case 's'  : //script/style Want to completely skip these
                    if(is_style(html_file, index, file_length)) {
                        char find_up[] = "</STYLE>";
                        char find_low[] = "</style>";
                        find_string(html_file, find_low, find_up, index, file_length);
                    }
                    else if(is_script(html_file, index, file_length)) {
                        char find_up[] = "</SCRIPT>";
                        char find_low[] = "</script>";
                        find_string(html_file, find_low, find_up, index, file_length);
                    }
                    else { //was not script or style tag, treat as ordinary tag
                        goto DEFAULT;
                    }
                    break;
                    
                case 'T'  :
                case 't'  :
                    //found <t, if <title, get it
                    if(is_title(html_file, index, file_length)) {
                        String type = "title";
                        get_words(html_file, index, file_length, type);
                    }
                    else {//was not <title, treat as ordinary tag
                        goto DEFAULT;
                    }
                    break;
                    
                DEFAULT:default : //ordinary tag. Just skip everything between <...>
                    char find[] = ">";
                    find_string(html_file, find, find, index, file_length);
            }
        }
        //Inside >...< and not script or style, so body. Get it.
        //ignore nothing
        else if(html_file[*index] == '\n' || html_file[*index] == ' ' || html_file[*index] == '\t' || html_file[*index] == '\r') {
            //do nothing
            (*index)++;
        }
        //grab the body text
        else {
            String type = "body";
            get_words(html_file, index, file_length, type);
        }
    }
    return 0;
}

bool LinkFinder::is_script(char *html_file, long *index, long file_length) {
    if(*index + 5 < file_length) {
        if(html_file[*index] == 's' && html_file[*index+1] == 'c' && html_file[*index+2] == 'r' && html_file[*index+3] == 'i' && html_file[*index+4] == 'p' && html_file[*index+5] == 't') {
            *index += 5;
            return true;
        }
        else if(html_file[*index] == 'S' && html_file[*index+1] == 'C' && html_file[*index+2] == 'R' && html_file[*index+3] == 'I' && html_file[*index+4] == 'P' && html_file[*index+5] == 'T') {
            *index += 5;
            return true;
        }
    }
    return false;
}

bool LinkFinder::is_style(char *html_file, long *index, long file_length) {
    if(*index + 4 < file_length) {
        if(html_file[*index] == 's' && html_file[*index+1] == 't' && html_file[*index+2] == 'y' && html_file[*index+3] == 'l' && html_file[*index+4] == 'e') {
            *index += 4;
            return true;
        }
        else if(html_file[*index] == 'S' && html_file[*index+1] == 'T' && html_file[*index+2] == 'Y' && html_file[*index+3] == 'L' && html_file[*index+4] == 'E') {
            *index += 4;
            return true;
        }
    }
    return false;
}

bool LinkFinder::is_title(char *html_file, long *index, long file_length) {
    if(*index + 4 < file_length) {
        if(html_file[*index] == 't' && html_file[*index+1] == 'i' && html_file[*index+2] == 't' && html_file[*index+3] == 'l' && html_file[*index+4] == 'e') {
            *index += 6;
            return true;
        }
        else if(html_file[*index] == 'T' && html_file[*index+1] == 'I' && html_file[*index+2] == 'T' && html_file[*index+3] == 'L' && html_file[*index+4] == 'E') {
            *index += 6;
            return true;
        }
    }
    return false;
}

void LinkFinder::find_string(char *html_file, char* find_lower, char* find_upper, long *index, long file_length) {
    int string_length = (int)strlen(find_lower);
    int counter = 0;
    while(*index < file_length && counter < string_length) {
        if(html_file[*index] == find_lower[counter]  || html_file[*index] == find_upper[counter]) {
            counter++;
        }
        else {
            counter = 0;
        }
        (*index)++;
    }
    return;
}

bool LinkFinder::find_link(char *html_file, char* find_lower, char* find_upper, long *index, long file_length) {
    int string_length = (int)strlen(find_lower);
    int counter = 0;
    while(*index < file_length && counter < string_length && html_file[*index] != '>') {
        if(html_file[*index] == find_lower[counter]  || html_file[*index] == find_upper[counter]) {
            counter++;
        }
        else {
            counter = 0;
        }
        (*index)++;
    }
    if(counter == string_length) {
        return true;
    }
    return false;
}

void LinkFinder::get_words(char *html_file, long *index, long file_length, String type) {
    Vector<String> full_anchor_text;
    while(*index < file_length && html_file[*index] != '<') {
        if(html_file[*index] != '\n' && html_file[*index] != '\t' && html_file[*index] != '\r' && html_file[*index] != ' ') {
            String word;
            while(*index < file_length && html_file[*index] != '\n' && html_file[*index] != '\t' && html_file[*index] != ' ' && html_file[*index] != '<' && html_file[*index] != '\r') {
                if(html_file[*index] != '"' && html_file[*index] != '(' && html_file[*index] != ')' && html_file[*index] != ',' && html_file[*index] != '.') {
                    if(isalpha(html_file[*index])) {
                        word += tolower(html_file[*index]);
                    }
                    else {
                        word += html_file[*index];
                    }
                }
                (*index)++;
            }//WHAT KIND OF WORDS DO WE WANT?
            if(strncmp(type.CString(), "anchor", 6) == 0) {
                full_anchor_text.push_back(word);
            }
            Index_object new_obj;
            new_obj.word = word;
            new_obj.type = type;//this is type
            new_obj.position = (int)Document.Words.size();
            Document.Words.push_back(new_obj);
        }
        else {
            (*index)++;
        }
    }
    if(strncmp(type.CString(), "anchor", 6) == 0) {
        Document.anchor_words.push_back(full_anchor_text);
    }
}

void LinkFinder::get_anchor_text(char *html_file, long *index, long file_length, long stop_index) {
    String type = "anchor";
    //Skip over all inner tags until we hit a's closing tag
    while(*index < file_length && *index < stop_index) {
        if(html_file[*index] == '<') {
            while(*index < file_length && html_file[*index] != '>') {
                (*index)++;
            }
            (*index)++;
        }
        get_words(html_file, index, file_length, type);
    }
    *index = stop_index;
    return;
}

void LinkFinder::reset_index(long *index, long reset_value) {
    *index = reset_value;
    return;
}

long LinkFinder::get_min(long index1, long index2) {
    return (index1 < index2) ? index1 : index2;
}

long LinkFinder::get_max(long index1, long index2) {
    return (index1 > index2) ? index1 : index2;
}

void LinkFinder::find_closing_a_tag(char *html_file, long *index, long file_length) {
    long reset_value = *index + 2;
    long parent_end_position = 0;
    
    //find first open tag
    if(find_open_tag(html_file, index, file_length)) {
        //found parent opening <
        //get length, then name
        int length = 2;
        while(*index < file_length && html_file[*index] != '>') {
            length++;
            (*index)++;
        }
        reset_index(index, *index - length + 2);
        char parent_tag[length];
        int i = 0;
        for(; i < length - 1; i++) {
            parent_tag[i] = html_file[*index];
            (*index)++;
        }
        parent_tag[i] = '\0';
        //find parent closing tag position
        parent_end_position = parent_tag_distance(html_file, parent_tag, index, file_length);
    }
    else {
        parent_end_position = 0;//ensures </a> is used
    }
    
    //reset file pointer
    reset_index(index, reset_value);
    
    //find closing a tag position
    char find_lowa[] = "</a>";
    char find_upA[] = "</A>";
    find_string(html_file, find_lowa, find_upA, index, file_length);
    
    //actual tag close will be min distance of parent and a tag
    long stop_index = get_min(parent_end_position, *index - 4);
    
    //in case of no parent, don't infinite loop
    if (stop_index <= reset_value) {
        stop_index = get_max(parent_end_position, *index - 4);
    }
    
    //reset file pointer
    reset_index(index, reset_value);
    
    //Get the anchor text
    get_anchor_text(html_file, index, file_length, stop_index);
    return;
}


long LinkFinder::parent_tag_distance(char *html_file, char* tag, long *index, long file_length) {
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
    while(*index < file_length) {
        if(html_file[*index] == end_tag[end_tag_counter]) {
            end_tag_counter++;
            if(end_tag_counter == string_length + 1) {
                if(!nested_tag) {
                    (*index) -= (strlen(end_tag) - 1);
                    return *index;
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
        if(html_file[*index] == tag[counter]) {
            counter++;
            if(counter == string_length) {
                nested_tag = true;
                counter = 0;
            }
        }
        else {
            counter = 0;
        }
        (*index)++;
    }
    return *index;
}

bool LinkFinder::find_open_tag(char *html_file, long *index, long file_length) {
    bool is_closing_tag = false;
    bool tag_found = false;
    int num_closing = 0;
    int num_opening = 0;
    while(*index > 0) {
        while(*index > 0 && *index < file_length && html_file[*index] != '>') { //find a tag
            if(html_file[*index] == '>') {
                tag_found = true;
            }
            (*index)--;
        }
        if(!tag_found) { //no parent tag found
            return false;
        }
        while(*index > 0 && html_file[*index] != '<') { //tag was found
            if(html_file[*index] == '/') {
                num_closing++; //uneven tag
                is_closing_tag = true; //is closing tag
            }
            (*index)--;
        }
        if(!is_closing_tag) {
            num_opening++; //Got here, then must be opening tag
        }
        is_closing_tag = false; //reset
        if(num_opening >= num_closing) { //check if tags are even
            return true;
        }
        (*index)++;
    }
    return false;//never get here
}