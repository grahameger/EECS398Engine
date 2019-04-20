
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
/*

int LinkFinder::parse(char* filename) {
    int fd = open(filename, O_RDONLY);
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
    while(*index < file_length) {//run until end of file
        if(html_file[*index] == '<') {
            (*index)++;
            switch(html_file[*index]) {
                case 'A'  :
                case 'a'  : //Link
                    if(html_file[*index+1] == ' ') {


*/
