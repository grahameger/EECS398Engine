//
//  constraint_solver.hpp
//  
//
//  Created by Bradley on 4/3/19.
//
//

#ifndef constraint_solver_hpp
#define constraint_solver_hpp
#include <stdio.h>
#include "string.h"
#include "index.hpp"


typedef size_t Location;
typedef size_t FileOffset;


class WordAttributes{
    String word;
    bool isBold;
    bool isItalicized;
    bool isHighlighted;
    bool isTitle;
};

class DocumentAttributes{
    int docID;
    int docLength;
    int numWords;
    String url;
};

class ISR{
public:
    
    Vector<String> parseQuery(String &query);
    ISR (String &query){
        parseQuery(query);
    }
    ISR* docEnd;
    virtual PostingListIndex* nextInstance();
    virtual PostingListIndex* nextDocument();
    virtual PostingListIndex* seekDocStart(Location docStart);
    virtual Location getDocStartLocation();
    virtual Location getDocEndLocation();
    virtual ISR* getDocISR();
};

class ISRWord : public ISR{
public:
    PostingListIndex* nextInstance();
    PostingListIndex* nextDocument();
    PostingListIndex* seekDocStart(Location docStart);
    PostingListIndex* getCurrentPost();
    Location getDocStartLocation();
    Location getDocEndLocation();
    ISR* getDocISR();
    String word;
    unsigned getDocumentCount();
    unsigned getNumberOfOccurences();
};


class ISROr : public ISR{
public:
    ISR** terms;
    unsigned numOfTerms;
    Location getStartLocation();
    Location getEndLocation();
    PostingListIndex* seekDocStart(Location target);
    PostingListIndex* nextInstance();
    PostingListIndex* nextDocument();
private:
    unsigned nearestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

class ISRAnd : public ISR{
public:
    ISR** terms;
    unsigned numOfTerms;
    PostingListIndex* seekDocStart(Location target);
    PostingListIndex* nextInstance();
private:
    unsigned nearestTerm, farthestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

class ISRPhrase : public ISR{
public:
    ISRWord** terms;
    unsigned numOfTerms;
    PostingListIndex* seekDocStart(Location target);
    PostingListIndex* nextInstance();
private:
    unsigned nearestTerm, farthestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

class ISREndDoc : public ISR {
    FileOffset docEnd;
    FileOffset docBegin;
};

//Advanced feature, may implement later
class ISRContainer : public ISR {
public:
    ISR** contained, excluded;
    ISREndDoc* endDoc;
    unsigned countContained, countExcluded;
    //Location Next();
    PostingListIndex* seek(Location target);
    PostingListIndex* next();
private:
    unsigned nearestTerm, farthestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

#endif /* constraint_solver_hpp */
