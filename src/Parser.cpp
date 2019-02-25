#include "Parser.hpp"

//default constructor
LinkFinder::LinkFinder() {}

//destructor
LinkFinder::~LinkFinder() {}

int LinkFinder::stripTags(const char* filename) {
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
    const char *html_file = (char*)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (html_file == MAP_FAILED){
        printf("mmap error\n");
        return -1;
    }
    if (close(fd)==-1) {
        printf("failed to close file (errno=%d)",errno);
        return -1;
    }
    std::regex reg("<(.*?)[\\s\\S]*?[\\s]*>");
    // ATOMICITY:
    // Choose a name for the temporary file.
    // Write the new content to a temporary file.
    // Flush the new content to disc.
    // Move the temporary file onto the original.
    //filename means pathname
    char tmp_pathname[strlen(filename)+2];
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
    write(fd, std::regex_replace((char*)html_file, reg, "").c_str(), strlen(html_file) + 1);
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
    return 0;
}

int LinkFinder::findLinks(const char* filename) {
    int start_positions [2000];//assuming at most 2000 links per page
    int end_positions [2000];
    int start_index = 0;
    int end_index = 0;
    bool a_tag = false;
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
    const char *html_file = (char*)mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (html_file == MAP_FAILED){
        printf("mmap error\n");
        return -1;
    }
    //Iterate through file. Get start and end positions of links
    int file_length = (int)strlen(html_file);
    for(int i = 0; i < file_length; i++) {
        //Found an <a or <A tag
        if(i+2 < file_length && html_file[i] == '<' && (html_file[i+1] == 'a' || html_file[i+1] == 'A') && html_file[i+2] == ' ') {
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
    //use regex on each substring to find link
    //**NOTE:Finds links if they exist i.e. href= isn't empty
    regex regx("(?:href=[\"']([^ >\"'\n\t]+))|(?:href=([^ >\"'\n\t]+))", std::regex_constants::icase);
    std::cmatch matches;
    char tmp_pathname[strlen(filename)+2];
    snprintf(tmp_pathname,sizeof(tmp_pathname),"%s~",filename);
    if (unlink(tmp_pathname)==-1) {
        if (errno!=ENOENT) {
            printf("failed to remove existing temporary file (errno=%d)",errno);
            return -1;
        }
    }
    if (close(fd)==-1) {
        printf("failed to close file (errno=%d)",errno);
        return -1;
    }
    mode_t default_mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
    fd = open(tmp_pathname,O_RDWR|O_CREAT|O_TRUNC,default_mode);
    if (fd==-1) {
        printf("failed to open new file for writing (errno=%d)",errno);
        return -1;
    }
    for(int i = 0; i < end_index; i++) {
        int length = end_positions[i] - start_positions[i] + 1;
        char substring[length];
        memcpy(substring, &html_file[start_positions[i]], end_positions[i] - start_positions[i]);
        substring[end_positions[i] - start_positions[i]] = '\0';
        if(std::regex_search(substring, matches, regx) ) {
            if(!matches[1].matched) {
                char add_space[strlen(matches[2].str().c_str()) + 2];
                strcpy(add_space, matches[2].str().c_str());
                strcat(add_space, " ");
                if(write(fd, add_space, strlen(add_space)) != strlen(add_space)) {
                    printf("Write didn't finish");
                    return -1;
                }
            }
            else {
                char add_space[strlen(matches[1].str().c_str()) + 2];
                strcpy(add_space, matches[1].str().c_str());
                strcat(add_space, " ");
                if(write(fd, add_space, strlen(add_space)) != strlen(add_space)) {
                    printf("Write didn't finish");
                    return -1;
                }
            }
        }
    }
    if (fsync(fd)==-1) {
        printf("failed to flush new file content to disc (errno=%d)", errno);
        return -1;
    }
    if (close(fd)==-1) {
        printf("failed to close new file (errno=%d)", errno);
        return -1;
    }
    char prepend[strlen(filename) + 6];
    strcpy(prepend, "links");
    if (rename(tmp_pathname, strcat(prepend, filename)) == -1) {
        printf("failed to move new file to final location (errno=%d)", errno);
        return -1;
    }
    return 0;
}