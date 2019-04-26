// Created by Graham Eger on 04/15/2019

#pragma once
#ifndef BLACKLIST_H_398
#define BLACKLIST_H_398

#include <string>
#include <set>

class Blacklist {
public:
    bool blacklisted(const std::string &url);
private:
    class BlacklistConstant {
        friend class Blacklist;
        BlacklistConstant();
        std::set<std::string> list;
    };
    inline static const BlacklistConstant blacklist;
};

#endif