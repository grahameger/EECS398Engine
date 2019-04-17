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
    Vector<ISRWord*> parseQuery(String &query);
    ISR (String &query){
        parseQuery(query);
    }
    ISR (){
        
    }
    Vector<ISR*> queries;
    ISR* docEnd;
    virtual Location nextInstance();
    virtual Location nextDocument();
    virtual Location seekDocStart(Location docStart);
    virtual Location getPostsStartLocation();
    virtual Location getPostsEndLocation();
    virtual ISR* getDocISR();
};

class ISRWord : public ISR{
public:
    String word;
    ISRWord(String wordToInsert){
        word = wordToInsert;
    }
    Location nextInstance();
    Location nextDocument();
    Location seekDocStart(Location docStart);
    Location getCurrentPost();
    Location getPostsStartLocation();
    Location getPostsEndLocation();
    ISR* getDocISR();
    unsigned getDocumentCount();
    unsigned getNumberOfOccurences();
};


class ISROr : public ISR{
public:
    Vector<ISR*> terms;
    ISROr(Vector<ISR> phrasesToInsert);
    void insert(ISR termToInsert);
    Location getPostsStartLocation();
    Location getPostsEndLocation();
    Location seekDocStart(Location target);
    Location nextInstance();
    Location nextDocument();
    unsigned numOfTerms = 0;
private:
    unsigned nearestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

class ISRAnd : public ISR{
public:
    Vector<ISR*> terms;
    ISRAnd(Vector<ISR> phrasesToInsert);
    unsigned numOfTerms = 0;
    PostingListIndex* seekDocStart(Location target);
    PostingListIndex* nextInstance();
private:
    unsigned nearestTerm, farthestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

class ISRPhrase : public ISR{
public:
    Vector<ISR*> terms;
    ISRPhrase(String phraseToStore);
    unsigned numOfTerms = 0;
    PostingListIndex* seekDocStart(Location target);
    PostingListIndex* nextInstance();
    //Make getter for nearestStartLocation, nearestEndLocation
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
    ISREndDoc* docEnd;
    unsigned countContained, countExcluded;
    //Location Next();
    PostingListIndex* seek(Location target);
    PostingListIndex* next();
private:
    unsigned nearestTerm, farthestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

#endif /* constraint_solver_hpp */
