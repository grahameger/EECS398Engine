#include <string>
#include <fstream>

#include "Blacklist.h"

bool Blacklist::blacklisted(const std::string &url) {
    return blacklist.list.find(url) != blacklist.list.end();
}

Blacklist::BlacklistConstant::BlacklistConstant() {
    std::ifstream blacklist("blacklist.txt");
    std::string host;
    while (getline(blacklist, host)) {
        list.insert(host);
    }
    blacklist.close();
}
