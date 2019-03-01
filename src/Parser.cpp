#include "Parser.hpp"

//default constructor
LinkFinder::LinkFinder() {}

//destructor
LinkFinder::~LinkFinder() {}

int LinkFinder::parse(char* filename) {
    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        perror("file open");
        return -1;
    };
    struct stat mystat = {};
    if (fstat(fd, &mystat)) {
        perror("fstat");
        return -1;
    };
    off_t file_size = mystat.st_size;
    char *html_file = (char*)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (html_file == MAP_FAILED){
        printf("mmap error\n");
        return -1;
    }
    if (close(fd)==-1) {
        printf("failed to close file (errno=%d)",errno);
        return -1;
    }
    long file_length = strlen(html_file);
    long num = 0;
    long *index = &num;
    bool anchor_text_not_img = false;
    while(*index < file_length) {//run until end of file
        if(html_file[*index] == '<') {
            (*index)++;
            switch(html_file[*index]) {
                case 'a'  : //Link
                    if(html_file[*index+1] == ' ') {
                        char find[] = "href=";
                        find_string(html_file, find, index, file_length);
                        std::cout << "link = ";
                        while(html_file[*index] != ' ' && html_file[*index] != '>') {
                            if(html_file[*index] != '"' && html_file[*index] != '\'') {
                                std::cout << html_file[*index];
                            }
                            (*index)++;
                        }
                        std::cout << std::endl;
                        
                        if(html_file[*index - 1] != '>') {
                            while(html_file[*index] != '>') {
                                (*index)++;
                            }
                        }
                        (*index)++;
                        while(html_file[*index] != '<') {
                            if(html_file[*index] != '\n' && html_file[*index] != ' ') {
                                anchor_text_not_img = true;
                            }
                            if(anchor_text_not_img) {
                                char anchor[] = "anchor";
                                get_anchor_word(html_file, index, file_length, anchor);
                                break;
                            }
                            (*index)++;
                        }
                        anchor_text_not_img = false;
                        char find2[] = "</a>";
                        find_string(html_file, find2, index, file_length);
                    }
                    //not a link, goto default
                    else {
                        goto DEFAULT;
                    }
                    break;
                    
                case 's'  : //script/style **Want to completely skip these
                    if(is_style(html_file, index, file_length)) {
                        char find[] = "</style>";
                        find_string(html_file, find, index, file_length);
                    }
                    else if(is_script(html_file, index, file_length)) {
                        char find[] = "</script>";
                        find_string(html_file, find, index, file_length);
                    }
                    else {
                        goto DEFAULT;
                    }
                    break;
                    
                case 't'  :
                    if(is_title(html_file, index, file_length)) {
                        char title[] = "title";
                        get_anchor_word(html_file, index, file_length, title);
                    }
                    else {
                        goto DEFAULT;
                    }
                    break;
                    
                DEFAULT:default : //not link or script or style tag, but still a tag just toss it
                    char find[] = ">";
                    find_string(html_file, find, index, file_length);
            }
        }
        else if(html_file[*index] == '\n' || html_file[*index] == ' ' || html_file[*index] == '\t') {
            //do nothing
            (*index)++;
        }
        else {//must be a word in text > word <
            int count = 0;
            long k = 0;
            k = *index; //2 character anchor text minimum
            while(k < file_length && count < 2 && html_file[k] != '<') {
                if(html_file[k] != ' ' && html_file[k] != '\n' && html_file[k] != '\t') {
                    count++;
                }
                k++;
            }
            if(count > 1) {
                std::cout << "body = ";
                while(html_file[*index] != '<') {
                    std::cout << html_file[*index];
                    (*index)++;
                }
                std::cout << std::endl;
            }
            else {
                (*index)++;
            }
        }
    }
    return 0;
}

void LinkFinder::get_anchor_word(char *html_file, long *index, long file_length, char* word) {
    if(*index + 1 < file_length && html_file[*index + 1] == '<') {
        //too small anchor text (only 1 char. worthless)
        (*index)++;
        return;
    }
    std::cout << word << " = ";
    while(*index < file_length && html_file[*index] != '<') {
        std::cout << html_file[*index];
        (*index)++;
    }
    std::cout << std::endl;
    return;
}


bool LinkFinder::is_script(char *html_file, long *index, long file_length) {
    if(*index + 5 < file_length) {
        if(html_file[*index] == 's' && html_file[*index+1] == 'c' && html_file[*index+2] == 'r' && html_file[*index+3] == 'i' && html_file[*index+4] == 'p' && html_file[*index+5] == 't') {
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
    }
    return false;
}

bool LinkFinder::is_title(char *html_file, long *index, long file_length) {
    if(*index + 4 < file_length) {
        if(html_file[*index] == 't' && html_file[*index+1] == 'i' && html_file[*index+2] == 't' && html_file[*index+3] == 'l' && html_file[*index+4] == 'e') {
            *index += 6;
            return true;
        }
    }
    return false;
}

void LinkFinder::find_string(char *html_file, char* find, long *index, long file_length) {
    int string_length = (int)strlen(find);
    int counter = 0;
    while(*index < file_length && counter < string_length) {
        if(html_file[*index] == find[counter]) {
            counter++;
        }
        else {
            counter = 0;
        }
        (*index)++;
    }
    return;
}


//------------------------------------------------------------------------------
// ATOMICITY:
// Choose a name for the temporary file.
// Write the new content to a temporary file.
// Flush the new content to disc.
// Move the temporary file onto the original.
//filename means pathname
/*  char tmp_pathname[strlen(filename)+2];
 snprintf(tmp_pathname,sizeof(tmp_pathname),"%s~",filename);
 if (unlink(tmp_pathname)==-1) {
 if (errno!=ENOENT) {
 printf("failed to remove existing temporary file (errno=%d)",errno);
 return -1;
 }
 }
 mode_t default_mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
 fd = open(tmp_pathname,O_RDWR|O_CREAT|O_TRUNC,default_mode);
 if (fd==-1) {
 printf("failed to open new file for writing (errno=%d)",errno);
 return -1;
 }
 char *p = " ";
 write(fd, kko, length);
 if (fsync(fd)==-1) {
 printf("failed to flush new file content to disc (errno=%d)",errno);
 return -1;
 }
 if (close(fd)==-1) {
 printf("failed to close new file (errno=%d)",errno);
 return -1;
 }
 if (rename(tmp_pathname,filename)==-1) {
 printf("failed to move new file to final location (errno=%d)",errno);
 return -1;
 }
 */