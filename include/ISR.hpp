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

class ISRWord{
public:
    //Vector of strings for separated queries
    Vector<String> queries;
    Vector<String> parseQuery(String &query);
    enum ISRType{ word, orr, aand, phrase };
    ISRType isrType;
    ISRWord (String &query){
        parseQuery(query);
    }
    ISRWord* DocumentEnd;
    virtual Post* next();
    virtual Post* nextDocument();
    virtual Post* seek(Location target);
    virtual Location getStartLocation();
    virtual Location getEndLocation();
    virtual ISRWord* getDocumentISR();
    String word;
    unsigned getDocumentCount();
    unsigned getNumberOfOccurences();
    Post* getCurrentPost();
};


class ISROr : public ISRWord{
public:
    ISRWord** terms;
    unsigned numOfTerms;
    Location getStartLocation();
    Location getEndLocation();
    Post* seek(Location target);
    Post* next();
    Post* nextDocument();
private:
    unsigned nearestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

class ISRAnd : public ISRWord{
public:
    ISR** terms;
    unsigned numOfTerms;
    Post* seek(Location target);
    Post* next();
private:
    unsigned nearestTerm, farthestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

class ISRPhrase : public ISRWord{
public:
    ISRWord** terms;
    unsigned numOfTerms;
    Post* seek(Location target);
    Post* next();
private:
    unsigned nearestTerm, farthestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

class ISREndDoc : public ISRWord {
    FileOffset docEnd;
    FileOffset docBegin;
};

//Advanced feature, may implement later
class ISRContainer : public ISRWord {
public:
    ISR** contained, excluded;
    ISREndDoc* endDoc;
    unsigned countContained, countExcluded;
    //Location Next();
    Post* seek(Location target);
    Post* next();
private:
    unsigned nearestTerm, farthestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

#endif /* constraint_solver_hpp */
