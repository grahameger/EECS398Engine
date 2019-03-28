#pragma once

#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <regex>

#include "constants.h"

namespace search {
    struct HTTPRequest {

        HTTPRequest(const std::string &url);

        // can optimize this later
        std::string filename() const;
        std::string requestString() const;
        void print();
        std::string   method;       // only GET implemented
        std::string   host;
        std::string   path;
        std::string   query;
        std::string   fragment;
        std::string   headers;
        std::string   protocol;
        int           port;         // note 0 defaults to 80
    };
}