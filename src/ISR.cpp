//
//  constraint_solver.cpp
//  
//
//  Created by Bradley on 4/3/19.
//
//

#include "ISR.hpp"
#include "Postings.h"

IsrWord::IsrWord( String word )
: validISR( true ), currentLocation(0){
    
    IsrInfo isrInfo = Postings::GetPostings( )->GetPostingList( word.CString( ) );
    nextPtr = isrInfo.nextPtr;
    
    if ( isrInfo.postingList )
        postingLists.push_back( isrInfo.postingList );
    else
        validISR = false;
    
    subBlocks.push_back( isrInfo.subBlock );
}


IsrWord::~IsrWord(){
    for( unsigned i = 0; i < postingLists.size(); i++ )
        delete postingLists[ i ];
    for( unsigned i = 0; i < subBlocks.size(); i++ )
        Postings::GetPostings()->MunmapSubBlock( subBlocks[i] );
}


Location IsrWord::nextInstance(){
    return SeekToLocation( 0 );
}


Location IsrWord::SeekToLocation (Location seekDestination){
    if ( !validISR || postingLists.empty( ) )
        return currentLocation = 0;
    
    for ( unsigned i = 0; i < postingLists.size( ); i++ )
    {
        Location posting = postingLists[ i ]->GetPosting( seekDestination );
        if ( posting != 0 )
            return currentLocation = posting;
    }
    
    while ( nextPtr != 0 )
    {
        IsrInfo isrInfo = Postings::GetPostings( )->GetPostingList( nextPtr );
        nextPtr = isrInfo.nextPtr;
        postingLists.push_back( isrInfo.postingList );
        subBlocks.push_back( isrInfo.subBlock );
        
        Location posting = isrInfo.postingList->GetPosting( seekDestination );
        if ( posting != 0 )
            return currentLocation = posting;
    }
    
    return currentLocation = 0;
}


Location IsrWord::CurInstance( ) const{
    return currentLocation;
}

//////////
//OR ISR//
//////////
ISROr::ISROr(Vector<Isr> phrasesToInsert){
    for (int i = 0; i < phrasesToInsert.size(); ++i){
        terms.push_back(phrasesToInsert[i]);
    }
    numOfTerms += phrasesToInsert.size();
}

Location ISROr::seek(Location target){
    //Move all ISRs to the first occurrence of their respective word at 'target' or later
    //Returns ULLONG_MAX if there is no match
    Location currentMin = ULLONG_MAX;
    for (int i = 0; i < numOfTerms; ++i){
        while (terms[i]->nextInstance() < target){
            Location nextLocation = terms[i]->nextInstance();
            //Terms[i] does not exist after target location
            if (nextLocation == ULLONG_MAX){
                return ULLONG_MAX;
            }
            //Terms[i] does exist after target location and is the first term to show up
            else if (nextLocation >= target && nextLocation < currentMin){
                currentMin = nextLocation;
            }
        }
    }
    return currentMin;
}

Location ISROr::nextInstance(){
    //Retrieve the next instance of an occurring term
    Location closestTerm = ULLONG_MAX;
    for (int i = 0; i < numOfTerms; ++i){
        if (terms[i]->nextInstance() < closestTerm){
            nearestTerm = i;
            closestTerm = terms[i]->nextInstance();
        }
    }
    return closestTerm;
}


/////////////////
//End of OR ISR//
/////////////////

///////////
//AND ISR//
///////////

ISRAnd::ISRAnd(Vector<Isr> phrasesToInsert){
    for (int i = 0; i < phrasesToInsert.size(); ++i){
        terms.push_back(phrasesToInsert[i]);
    }
    numOfTerms += phrasesToInsert.size();
}


Location ISRAnd::seek(Location target){
    //TODO
    // 1. Seek all ISRs to the first occurrence beginning at the target location.
    // 2. Pick the furthest term and attempt to seek all the other terms to the
    //first location beginning where they should appear relative to the furthest term.
    // 3. If any term is past the desired location, return to step 2.
    // 4. If any ISR reaches the end, there is no match.
    Location currentMin = ULLONG_MAX;
    for (int i = 0; i < numOfTerms; ++i){
        while (terms[i]->nextInstance() < target){
            Location nextLocation = terms[i]->nextInstance();
            //Terms[i] does not exist after target location
            if (nextLocation == ULLONG_MAX){
                return ULLONG_MAX;
            }
            //Terms[i] does exist after target location and is the first term to show up
            else if (nextLocation >= target && nextLocation < currentMin){
                currentMin = nextLocation;
            }
        }
    }
    return currentMin;
}

//////////////////
//End of AND ISR//
//////////////////

//////////////
//Phrase ISR//
//////////////
ISRPhrase::ISRPhrase(String phraseToStore){
    String currWord = "";
    for (int i = 0; i < phraseToStore.Size(); ++i){
        if (isalpha(phraseToStore[i])){
            currWord += &phraseToStore[i];
        }
        else {
            IsrWord* subPhrase = new IsrWord(currWord);
            currWord = "";
            terms.push_back(subPhrase);
        }
    }
}

Location ISRPhrase::seek(Location target){
    Vector<Location> termLocations;
    for (int i = 0; i < numOfTerms; ++i){
        while (terms[i]->nextInstance() < target){
            Location nextLocation = terms[i]->nextInstance();
            //No phrase exists
            if (nextLocation == ULLONG_MAX){
                return ULLONG_MAX;
            }
            else if (nextLocation > target){
                //Found the first instance of the first term appearing after target
                if (i == 0){
                    termLocations.push_back(nextLocation);
                }
                else {
                    //Found a potential phrase match
                    if (termLocations[i-1] == nextLocation - 1){
                        termLocations.push_back(nextLocation);
                    }
                    //Phrase match invalid, restart search
                    else {
                        i = 0;
                        while (!termLocations.empty()){
                            termLocations.pop_back();
                        }
                    }
                }
            }
        }
    }
    return termLocations[0];
}


