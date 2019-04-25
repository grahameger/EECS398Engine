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
#include "expression.h"
#include "String.h"

typedef unsigned long long Location;
typedef size_t FileOffset;

namespace IsrGlobals
   {
   const Location IsrSentinel = 0;
   //TODO: CHange to 1
   const Location IndexStart = 1;
   }

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

class Isr{
public:
    Isr (){
        
    }
    //Isr pointer to the end of the current doc
    Isr* docEnd;
    
    //Find next instance of a term
    virtual Location nextInstance();
    
    //Similar to nextDocument, finds the first occurrence of a term just at 'target' or later
    virtual Location SeekToLocation(Location target);
    
    //First number in the posting list of a term
    virtual Location getClosestStartLocation();
    
    //Last number in the posting list of a term
    virtual Location getClosestEndLocation();
    
    //Returns whatever document you're looking at
    virtual Isr* getDocIsr();
    
    virtual Location CurInstance();
};

class IsrWord : public Isr {
public:
    IsrWord( String word );
    void SetLocations( Vector<Location>& matchesIn );
    ~IsrWord( );
    
    Location nextInstance( );
    Location SeekToLocation( Location seekDestination = 0 );
    Location GetCurrentLocation( ) const;
    
    operator bool( ) const;
    
private:
    bool validIsr;
    Location currentLocation;
    
    unsigned nextPtr;
    Vector<Location> matches;
    unsigned curInd;
    bool hasNextInstance();
};


class IsrOr : public Isr{
public:
    //List of Isrs that we keep track of
    Vector<Isr*> terms;
    
    //Variable to keep track of how many terms are in 'terms' (because resize/reserve isn't implemented)
    unsigned numOfTerms = 0;
    
    //Constructor for IsrOr, MUST be in a vector<Isr> format, otherwise it wont compile
    IsrOr(Vector<Isr> phrasesToInsert);
    
    //Points to the closest 'beginning of page'
    Location getClosestStartLocation(){
        return nearestStartLocation;
    }
    
    //Points to the closest 'end of page'
    Location getClosestEndLocation(){
        return nearestEndLocation;
    }
    
    //Move all Isrs to the first occurrence of their respective word at 'target' or later
    //Returns ULLONG_MAX if there is no match
    Location SeekToLocation(Location target);
    //TODO:
    // 1. Seek all the Isrs to the first occurrence beginning at
    //    the target location.
    // 2. Move the document end Isr to just past the furthest
    //    word, then calculate the document begin location.
    // 3. Seek all the other terms to past the document begin.
    // 4. If any term is past the document end, return to step 2
    // 5. If any Isr reaches the end, there is no match.
    
    //Find the next instance of ANY of the words in 'terms'
    Location nextInstance();
    
    //Seek all Isrs to the first occurrence JUST PAST the end of the current document
    Location nextDocument(){
        return SeekToLocation(docEnd->getClosestEndLocation() + 1);
    }
private:
    //
    unsigned nearestTerm = 99999;
    Location nearestStartLocation, nearestEndLocation;
};

class IsrAnd : public Isr{
public:
    //Container for terms
    Vector<Isr*> terms;
    
    //Keeps track of how many terms we have
    unsigned numOfTerms = 0;
    
    //Constructor for IsrAnd
    IsrAnd(Vector<Isr> phrasesToInsert);
    
    Location SeekToLocation(Location target);
        //TODO:
        // 1. Seek all the Isrs to the first occurrence beginning at
        //    the target location.
        // 2. Move the document end Isr to just past the furthest
        //    word, then calculate the document begin location.
        // 3. Seek all the other terms to past the document begin.
        // 4. If any term is past the document end, return to step 2
        // 5. If any Isr reaches the end, there is no match.
    
    //Finds next instance of all terms in a page
    Location nextInstance(){
        return SeekToLocation(nearestStartLocation + 1);
    }
    
    //Points to the closest 'beginning of page'
    Location getClosestStartLocation(){
        return nearestStartLocation;
    }
    
    //Points to the closest 'end of page'
    Location getClosestEndLocation(){
        return nearestEndLocation;
    }
    
private:
    unsigned nearestTerm, farthestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

class IsrPhrase : public Isr{
public:
    //Container for terms
    Vector<Isr*> terms;
    
    //Keeps track of how many terms we have
    unsigned numOfTerms = 0;
    
    //Constructor for IsrPhrase
    IsrPhrase(String phraseToStore);
    
    //Finds next instance after target location
    Location seek(Location target);
    
    //Finds next instance of phrase match
    Location nextInstance(){
        return seek(nearestStartLocation + 1);
    }

private:
    unsigned nearestTerm, farthestTerm;
    Location nearestStartLocation, nearestEndLocation;
};

/*
//ULONG_MAX for failure
//Should have an enddoc Isr pointing to end of pages

class IsrEndDoc : IsrWord{
public:
    IsrEndDoc( );
};

//Advanced feature, may implement later
class IsrContainer : public Isr {
public:
    Isr** contained, excluded;
    IsrEndDoc* docEnd;
    unsigned countContained, countExcluded;
    //Location Next();
    Location seek(Location target);
    Location next();
private:
    unsigned nearestTerm, farthestTerm;
    Location nearestStartLocation, nearestEndLocation;
};
*/

class IsrEndDoc : public IsrWord
   {
   public:
      DocumentAttributes GetDocInfo();
   }

#endif /* constraint_solver_hpp */
