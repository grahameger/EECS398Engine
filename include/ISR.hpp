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
#include <string>

class WordAttributes{
    //Constructor
    WordAttributes(std::string input);
    //Member variable
    std::string word;
};
class DocumentAttributes{
    //Constructor
    DocumentAttributes(int docNum);
    int docID;
};

typedef size_t Location;
typedef size_t FileOffset;
typedef union Attributes {
    DocumentAttributes document;
    WordAttributes word;
};

//Gets rid of circular dependencies
class Post;

class ISR{
    public:
    ISR* DocumentEnd;
    virtual Post* next();
    virtual Post* nextDocument();
    virtual Post* seek(Location target);
    virtual Location getStartLocation();
    virtual Location getEndLocation();
    virtual ISR* getDocumentISR();
};

class ISREndDoc : public ISR {
    FileOffset docEnd;
    FileOffset docBegin;
};

class ISRContainer : public ISR {
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

class ISRWord : public ISR{
public:
    unsigned getDocumentCount();
    unsigned getNumberOfOccurences();
    virtual Post* getCurrentPost();
};

class ISRPhrase : public ISR{
public:
    ISR** terms;
    unsigned numOfTerms;
    Post* seek(Location target);
    Post* next();
private:
    unsigned nearestTerm, farthestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

class ISROr : public ISR{
public:
    ISR** terms;
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

class ISRAnd : public ISR{
public:
    ISR** terms;
    unsigned numOfTerms;
    Post* seek(Location target);
    Post* next();
private:
    unsigned nearestTerm, farthestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

//ALL CLASSES BELOW ARE TEMP CLASSES UNTIL INDEX IS FINISHED

//Temp post class
class Post {
public:
    virtual Location getStartLocation();
    virtual Location getEndLocation();
    virtual Attributes getAttributes();
    virtual Post *next();
    virtual ISR *getISR();
};

//Temp dictionary class
class Dictionary {
public:
    ISR* openISR(char* token);
    Location getNumberOfWords();
    Location getNumberOfUniqueWords();
    Location getNumberOfDocuments();
};

//Temp postinglist class
class PostingList{
public:
    virtual Post* Seek(Location);
private:
    struct PostingListIndex{
        FileOffset offset;
        Location postLocation;
    };
    PostingListIndex* index;
    virtual char* getPostingList();
};

//Temp index class
class Index{
public:
    Location WordsInIndex, DocumentsInIndex, LocationsInIndex, MaximumLocation;
    ISRWord* OpenISRWord(char*word);
    ISRWord* OpenISREndDoc();
};


#endif /* constraint_solver_hpp */
