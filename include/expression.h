/*
 * expression.h
 *
 * Class declarations for expressions
 *
 */
#pragma once
#ifndef EXPRESSION_H_
#define EXPRESSION_H_
#include <stdint.h>
#include <vector>
#include <cstddef>
#include "String.hpp"
#include "index.hpp"
using namespace std;

typedef unsigned long long Location;
typedef size_t FileOffset;

//Not really sure what attributes are used for but they were in the lecture slides
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
    Vector<ISR*> parseQuery(String &query);
    ISR (String &query){
        parseQuery(query);
    }
    ISR (){}
    
    Vector<ISR*> queries;
    //ISR pointer to the end of the current doc
    ISR* docEnd;
    
    //Find next instance of a term
    virtual Location nextInstance();
    
    //Finds the first occurrence of a term just pass the end of the document
    virtual Location nextDocument();
    
    //Similar to nextDocument, finds the first occurrence of a term just at 'target' or later
    virtual Location seek(Location target);
    
    //First number in the posting list of a term
    virtual Location getClosestStartLocation();
    
    //Last number in the posting list of a term
    virtual Location getClosestEndLocation();
    
    //Returns whatever document you're looking at
    virtual ISR* getDocISR();
};

class ISRWord : public ISR{
public:
    //Word to be looked up
    String word;
    
    //Basic constructor - does nothing
    ISRWord(){};
    
    //Custom constructor
    ISRWord(String wordToInsert){
        word = wordToInsert;
    }
    //Points to the current location on a document
    Location getCurrentPost();
    
    //Gets number of documents the word appears in
    unsigned getDocumentCount();
    
    //Gets number of times the word appears in index
    unsigned getNumberOfOccurences();
    
    //Note: ISRWord will use its parent class's functions for nextInstance,
    //nextDocument, seek, getPostsStart, getPostsEnd, getDocISR
};



class ISRPhrase : public ISR{
public:
    void addTerm(ISR *);
    
    //Keeps track of how many terms we have
    unsigned numOfTerms = 0;
    
    ISRPhrase(){}
    //Constructor for ISRPhrase
    ISRPhrase(String phraseToStore);
    
    Location seek(Location target);
    //TODO
    // 1. Seek all ISRs to the first occurrence beginning at the target location.
    // 2. Pick the furthest term and attempt to seek all the other terms to the
    //first location beginning where they should appear relative to the furthest term.
    // 3. If any term is past the desired location, return to step 2.
    // 4. If any ISR reaches the end, there is no match.
    
    //Finds next instance of phrase match
    Location nextInstance(){
        return seek(nearestStartLocation + 1);
    }
    
private:
    unsigned nearestTerm, farthestTerm;
    Location nearestStartLocation, nearestEndLocation;
    //List of ISRs that we keep track of
    Vector<ISR*> terms;
};



class ISRAnd : public ISR{
public:
    void addTerm(ISR *);
    
    //Keeps track of how many terms we have
    unsigned numOfTerms = 0;
    
    ISRAnd() {}
    //Constructor for ISRAnd
    ISRAnd(Vector<ISR> phrasesToInsert);
    
    Location seek(Location target);
    //TODO:
    // 1. Seek all the ISRs to the first occurrence beginning at
    //    the target location.
    // 2. Move the document end ISR to just past the furthest
    //    word, then calculate the document begin location.
    // 3. Seek all the other terms to past the document begin.
    // 4. If any term is past the document end, return to step 2
    // 5. If any ISR reaches the end, there is no match.
    
    //Finds next instance of all terms in a page
    Location nextInstance(){
        return seek(nearestStartLocation + 1);
    }
    
private:
    unsigned nearestTerm, farthestTerm;
    Location nearestStartLocation, nearestEndLocation;
    //List of ISRs that we keep track of
    Vector<ISR*> terms;
};



class ISROr : public ISR{
public:
    void addTerm(ISR *);
    
    
    //Variable to keep track of how many terms are in 'terms' (because resize/reserve isn't implemented)
    unsigned numOfTerms = 0;
    
    ISROr(){}
    //Constructor for ISROr, MUST be in a vector<ISR> format, otherwise it wont compile
    ISROr(Vector<ISR> phrasesToInsert);
    
    //Points to the closest 'beginning of page'
    Location getClosestStartLocation(){
        return nearestStartLocation;
    }
    
    //Points to the closest 'end of page'
    Location getClosestEndLocation(){
        return nearestEndLocation;
    }
    
    //Move all ISRs to the first occurrence of their respective word at 'target' or later
    //Returns ULLONG_MAX if there is no match
    Location seek(Location target);
    //TODO:
    // 1. Seek all the ISRs to the first occurrence beginning at
    //    the target location.
    // 2. Move the document end ISR to just past the furthest
    //    word, then calculate the document begin location.
    // 3. Seek all the other terms to past the document begin.
    // 4. If any term is past the document end, return to step 2
    // 5. If any ISR reaches the end, there is no match.
    
    //Find the next instance of ANY of the words in 'terms'
    Location nextInstance();
    
    //Seek all ISRs to the first occurrence JUST PAST the end of the current document
    Location nextDocument(){
        return seek(docEnd->getPostsEndLocation() + 1);
    }
private:
    //
    //List of ISRs that we keep track of
    Vector<ISR*> terms;
    unsigned nearestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

#endif /* EXPRESSION_H_ */
