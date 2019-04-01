//
//  index.hpp
//  inverted index for a search engine that is memory efficient
//
//  Created by Jake C on 3/17/19.
//  Copyright © 2019 Jake C. All rights reserved.
//
#pragma once
#ifndef __index_hpp__
#define __index_hpp__

#include <stdio.h>
#include <map>
#include <iostream>
#include "vector.h"
#include <string>

struct Dictionary {
    int numOfTokensInIndex;
    int numOfUniqueTokensInIndex;//num of unique words
    int numOfDocumentsInIndex;
    typedef std::map<Token, Posting> Index;//find offset into posting list from token(aka word or decorated word)
    //Will hold a mapping to posting list from all the words we'll find in our documents
};

struct Post {
    long docID;//individual document
    int deltaFromPrevPost;
    string typeSpecificData;
    
};

struct PostingList {
    string commonHeader;
    string TypeSpecificData;
    int index;
    vector<Post> posts; //set of document ids. makes up the postings list
    string sentinel;
};

class InvertedIndex {
private:
    Dictionary dictionary;
    vector<PostingList> postingListList;
public:
    
    void insert(string word, int docID) {
        map.insert(word, DocID);
    }
    
    //IRS methods
    void first(t) {
        return;
    }
    
    void last(t) {
        return;
    }
    void next(t, current) {
        return;
    }
    
    void prev(t, current) {
        return;
    }
};

#endif // __index_hpp__