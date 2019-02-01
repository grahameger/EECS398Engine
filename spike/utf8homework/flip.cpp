//
//  main.cpp
//  utf8-practice
//
//  Created by Graham Eger on 2/1/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include "Utf8.h"
#include <locale>
#include <codecvt>
#include <sstream>

enum Encoding {
   UTF32BE,
   UTF32LE,
   UTF16BE,
   UTF16LE,
   UTF8BOM,
   OTHER
};

class FileAccessor {
public:
   size_t filesize_bytes;
   int fd;
   char * data_start;
   
   FileAccessor(const std::string &filename) {
      // we're fucking memory mapping a file
      int fd, ret;
      struct stat st;
      if ((fd = open(filename.c_str(), O_RDONLY)) < 0) {
         perror("Error in file opening");
         exit(1);
      }
      if ((ret = fstat(fd,&st)) < 0) {
         perror("Error in fstat");
         exit(1);
      }
      filesize_bytes = st.st_size;
      /* len_file having the total length of the file(fd) */
      if ((data_start = (char*)mmap(NULL, filesize_bytes, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
         perror("Error in mmap");
         exit(1);
      }
   }
   ~FileAccessor() {
      close(fd);
   }
   void * get_ptr() { return data_start; }
   size_t size() {
      return filesize_bytes;
   }
};

static bool ReadBOM(char * start) {
   return (*start && *(start + 1) && *(start + 2)) &&
          (start[0] == '\xEF' && start[1] == '\xBB' && start[2] == '\xBF');
}

// 0x00, 0x00, 0xfe, 0xff -- The file is almost certainly UTF-32BE
// 0xff, 0xfe, 0x00, 0x00 -- The file is almost certainly UTF-32LE
// 0xfe, 0xff,  XX,   XX     -- The file is almost certainly UTF-16BE
// 0xff, 0xfe,  XX,   XX (but not 00, 00) -- The file is almost certainly UTF-16LE
// 0xef, 0xbb, 0xbf,  XX   -- The file is almost certainly UTF-8 With a BOM
static Encoding GuessEncoding(uint8_t * start, size_t len) {
   if (len < 4) {
      return OTHER;
   } else {
      switch (start[0]) {
         case 0x00:
            switch (start[1]) {
               case 0x00:
                  switch (start[2]) {
                     case 0xFE:
                        switch (start[3]) {
                           case 0xFF:
                              return UTF32BE;
                           default:
                              return OTHER;
                        }
                     default:
                        return OTHER;
                  }
               default:
                  return OTHER;
            }
            break;
         case 0xFF:
            switch (start[1]) {
               case 0xFE:
                  if (start[2] == 0x00 && start[3] == 0x00) {
                     return UTF32LE;
                  } else {
                     return UTF16LE;
                  }
               default:
                  return OTHER;
            }
            break;
         case 0xFE:
            if (start[1] == 0xFF) {
               return UTF16BE;
            }
         case 0xEF:
            if (start[1] == 0xBB && start[2] == 0xBF) {
               return UTF8BOM;
            }
         default:
            return OTHER;
            break;
      }
   }
}

// convert UTF8/ASCII to Unicode.
// convert Unicode to UTF8
int main(int argc, const char * argv[]) {
   // error handling
   if (argc != 2) {
      fprintf(stdout, "Usage: flip <filename>\n");
      return 0;
   }
   FileAccessor file = FileAccessor(std::string(argv[1]));
   void * start = file.get_ptr();
   // check starting 4 bytes to determine what we have
   Encoding encoding = GuessEncoding((uint8_t*)start, file.size());
   // move it forward if it's just ASCII / Unicode
   if (encoding == OTHER || encoding == UTF8BOM) {
      start = (uint8_t*)start + 3;
   }
   
   if (encoding == UTF16LE || UTF16BE) {
      // should take each 16 bit segment and convert it to UTF32 internal
      // representation then encode as UTF8, and write it to stdout
      
   }
//   else if (encoding == UTF8BOM || encoding == OTHER) {
//
//   }
   
   return 0;
}
