#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <algorithm>
#include <mutex>
#include "String.h"
#include <signal.h>
#include <stdlib.h>
#include <unordered_set>
#include <stdio.h>
#include <getopt.h>
#include <thread>

#include "crawler.h"
#include "PersistentHashMap.h"
#include "httpRequest.h"
#include "http.h"
#include "Parser.hpp"
#include "threading.h"

#include <deque>
#include "PersistentHashMap.h"
#include "Parser.hpp"
#include "threading.h"
#include "index.h"

int main() {
  std::vector<std::string> examples({
    "g:h",
    "g",
    "./g",
    "g/",
    "/g",
    "//g",
    "?y",
    "g?y",
    "#s",
    "g#s",
    "g?y#s",
    ";x",
    "g;x",
    "g;x?y#s",
    ".",
    "./",
    "..",
    "../",
    "../g",
    "../..",
    "../../",
    "../../g"
  });
  std::string base_uri = "http://a/b/c/d;p?q";
  for (auto i : examples) {
    std::cout << i << '\t' << "= " << search::HTTPClient::resolveRelativeUrl(base_uri.c_str(), i.c_str()) << std::endl;
  }
}