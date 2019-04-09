#pragma once

#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <regex>

#include "constants.h"

namespace search {
    struct HTTPRequest {

        HTTPRequest(std::string url);

        // can optimize this later
        std::string filename() const;
        std::string requestString() const;
        std::string uri() const;
        bool robots() const;
        void print() const;
        bool goodExtension() const;
        std::string   method;       // only GET implemented
        std::string   host; // hostname
        std::string   path; // easy enough
        std::string   query; // ? stuff
        std::string   fragment; // # stuff
        std::string   headers; // TODO
        std::string   scheme; // http://, https:// etc
        int           port;         // note 0 defaults to 80

        // if you update one, update both.
        inline static const std::string BAD_EXTENSIONS[] = {".png",
                                                            ".jpg",
                                                            ".jpeg",
                                                            ".gif",
                                                            ".tiff",
                                                            ".bmp",
                                                            ".webm",
                                                            ".mp4",
                                                            ".webp",
                                                            ".js",
                                                            ".css",
                                                            ".mp3",
                                                            ".m4a",
                                                            ".ogg",
                                                            ".flac"};
        inline static const size_t NUM_BAD_EXTENSIONS = 15;
    };
}