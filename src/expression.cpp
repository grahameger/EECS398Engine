/*
 * expression.cpp
 *
 * Class implementations for expression functionality
 *
 */

#include "expression.h"
#include "Postings.h"

ISRWord::ISRWord( String word )
: validISR( true ), currentLocation(0){
    
    IsrInfo isrInfo = Postings::GetPostings( )->GetPostingList( word.CString( ) );
    nextPtr = isrInfo.nextPtr;
    
    if ( isrInfo.postingList )
        postingLists.push_back( isrInfo.postingList );
    else
        validISR = false;
    
    subBlocks.push_back( isrInfo.subBlock );
}


ISRWord::~ISRWord(){
    for( unsigned i = 0; i < postingLists.size(); i++ )
        delete postingLists[ i ];
    for( unsigned i = 0; i < subBlocks.size(); i++ )
        Postings::GetPostings()->MunmapSubBlock( subBlocks[i] );
}


Location ISRWord::nextInstance(){
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


Location ISRWord::CurInstance( ) const{
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

void ISROr::addTerm(ISR * phrase) {
    terms.push_back(phrase);

}

Location ISROr::seek(Location target){
    //Move all ISRs to the first occurrence of their respective word at 'target' or later
    //Returns ULLONG_MAX if there is no match
    Location nearestStartLocationTracker = ULLONG_MAX;
    Location nearestEndLocationTracker = 0;
    Location nearestTermTracker = 0;
    for (int i = 0; i < numOfTerms; ++i){
        while (terms[i]->nextInstance() < target){
            Location nextLocation = terms[i]->nextInstance();
            //All terms do not exist after target location
            if (nextLocation == ULLONG_MAX && i == numOfTerms - 1){
                return ULLONG_MAX;
            }
            //Terms[i] does exist after target location and is the first term to show up
            else if (nextLocation >= target && nextLocation < nearestStartLocationTracker){
                nearestStartLocationTracker = nextLocation;
                nearestTermTracker = i;
            }
            //Terms[i] does exist after target location and is the last term to show up
            else if (nextLocation >= target && nextLocation > nearestEndLocationTracker){
                nearestEndLocationTracker = nextLocation;
            }
        }
    }
    nearestStartLocation = nearestStartLocationTracker;
    nearestEndLocation = nearestEndLocationTracker;
    nearestTerm = nearestTermTracker;
    return nearestStartLocation;
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

void ISRAnd::addTerm(ISR * phrase) {
    terms.push_back(phrase);
}

Location ISRAnd::seek(Location target){
    //TODO
    // 1. Seek all ISRs to the first occurrence beginning at the target location.
    // 2. Pick the furthest term and attempt to seek all the other terms to the
    //first location beginning where they should appear relative to the furthest term.
    // 3. If any term is past the desired location, return to step 2.
    // 4. If any ISR reaches the end, there is no match.
    Location currentMin = ULLONG_MAX;
    Vector<Location>docTracker;
    for (int i = 0; i < numOfTerms; ++i){
        while (terms[i]->nextInstance() < target){
            Location nextLocation = terms[i]->nextInstance();
            IsrWord nextPage("");
            Location pageEnd = nextPage.SeekToLocation(nextLocation);
            //Terms[i] does not exist after target location
            if (nextLocation == ULLONG_MAX){
                return ULLONG_MAX;
            }
            //Terms[i] does exist after target location and is the first term to show up
            if (nextLocation > target){
                if (nextLocation < currentMin){
                    currentMin = nextLocation;
                    nearestTerm = i;
                    nearestStartLocation = currentMin;
                }
                
                //Page matches, or we are pushing back page of first term
                if (i == 0 || docTracker[i] == docTracker[i-1]){
                    docTracker.push_back(pageEnd);
                }
                //Pages of terms do not match, reset our search
                else {
                    i = 0;
                    while (!docTracker.empty()){
                        docTracker.pop_back();
                    }
                    
                }
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
    
    if (termLocations.empty()){
        return ULLONG_MAX;
    }
    return termLocations[0];
}
