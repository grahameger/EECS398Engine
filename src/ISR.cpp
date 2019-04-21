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

//////////////
//EndDoc ISR//
//////////////
IsrEndDoc::IsrEndDoc( ) : IsrWord( "" ){
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
    //Algorithm
    // 1. Seek all ISRs to the first occurrence beginning at the target location.
    // 2. Loop through all the terms and return the term location which is smallest
    //Returns 0 if there is no match
    Location closestTerm = ULLONG_MAX;
    //Step 1: seek all ISRs to first occurrence after target location
    for (int i = 0; i < numOfTerms; ++i){
        //Traverse each term's posting list until it goes past 'target' or reaches the end
        while (terms[i]->CurInstance() < target){
            //We've looped thru all terms, and nextInstance of the last term DNE,
            //so no term exists after location target
            if (terms[i]->nextInstance() == 0 && i == numOfTerms - 1 && closestTerm == ULLONG_MAX){
                return 0;
            }
            Location nextLocation = terms[i]->nextInstance();
            //Update closestTerm, the term that is closest to and > target
            if (nextLocation < closestTerm && nextLocation >= target){
                closestTerm = nextLocation;
                nearestTerm = i;
            }
        }
    }
    //nearestStartLocation is the first term that appears in the OR ISR
    nearestStartLocation = closestTerm;
    //Update nearestEndLocation, the end of the page of nearestStartLocation
    IsrWord endPage("");
    nearestEndLocation = endPage.SeekToLocation(closestTerm);
    return closestTerm;
    
}



Location ISROr::nextInstance(){
    
    //Retrieve the next instance of the first occurring term
    //Case 1: nearestTerm is initialized (because we initialize nearestTerm to 99999)
    //and is an index to the closest term
    if (nearestTerm != 99999){
        terms[nearestTerm]->nextInstance();
        Location closestTerm = ULLONG_MAX;
        for (int i = 0; i < numOfTerms; ++i){
            if (terms[i]->CurInstance() < closestTerm && terms[i]->CurInstance() != 0){
                closestTerm = terms[i]->CurInstance();
                nearestTerm = i;
            }
            //There are no next instances of any of the terms
            if (terms[i]->CurInstance() == 0 && i == numOfTerms - 1 && closestTerm == ULLONG_MAX){
                return 0;
            }
        }
        nearestStartLocation = closestTerm;
        IsrWord endPage("");
        nearestEndLocation = endPage.SeekToLocation(closestTerm);
        return closestTerm;
    }
    //Case 2: nearestTerm is not initialized yet, this is the first call of nextInstance()
    //so we must find the term that is closest to the start of a posting list
    else {
        return seek(0);
    }

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
    //Algorithm
    // 1. Seek all ISRs to the first occurrence beginning at the target location.
    // 2. Pick the furthest term and attempt to seek all the other terms to the
    //first location beginning where they should appear relative to the furthest term.
    // 3. If any term is past the desired location, return to step 2.
    // 4. If any ISR reaches the end, there is no match.
    Vector<Pair<IsrWord, Location>>docTracker;
    
    //Step 1: seek all ISRs to first occurrence after target location
    for (int i = 0; i < numOfTerms; ++i){
        //Traverse each term's posting list until it goes past 'target' or reaches the end
        while (terms[i]->CurInstance() < target){
            //None of the posting list terms are on the same page
            if (terms[i]->nextInstance() == 0){
                return 0;
            }
            Location nextLocation = terms[i]->nextInstance();
            IsrWord nextPage("");
            Location pageEnd = nextPage.SeekToLocation(nextLocation);
            Pair<IsrWord, Location> toInsert(terms[i], pageEnd);
            docTracker.push_back(toInsert);
        }
        
    }
    
    Location latestPage = 0;
    bool allSamePage = false;
    while (!allSamePage){
        //Step 2: Find the furthest term's page
        for (int i = 0; i < docTracker.size(); ++i){
            if (docTracker[i].second > latestPage){
                latestPage = docTracker[i].second;
            }
        }
        
        //Step 3: Scan other pages, if other pages are before furthest term, move them forward
        bool pageAltered = false;
        for (int i = 0; i < docTracker.size(); ++i){
            if (docTracker[i].second < latestPage){
                while (docTracker[i].first->CurInstance() < latestPage){
                    //Step 4: Check if any pages reach the end
                    if (docTracker[i].first->nextInstance() == 0){
                        return 0;
                    }
                    //Update the IsrWord index and latest page
                    docTracker[i].first = docTracker[i].first->nextInstance();
                    IsrWord nextPage("");
                    Location valToCompare = nextPage.SeekToLocation(docTracker[i].first);
                    if (valToCompare != docTracker[i].second){
                        pageAltered = true;
                    }
                    docTracker[i].second = nextPage.SeekToLocation(docTracker[i].first);
                }
            }
        }
        //Go back to step 2 if we moved any ISRs
        if (!pageAltered){
            allSamePage = true;
        }
    }
    
    Location earliestPost = ULLONG_MAX;
    Location latestPost = 0;
    for (int i = 0; i < docTracker.size(); ++i){
        if (docTracker[i].first->CurInstance() < earliestPost){
            earliestPost = docTracker[i].first->CurInstance();
            nearestTerm = i;
        }
        if (docTracker[i].first->CurInstance() > latestPost){
            latestPost = docTracker[i].first->CurInstance();
            farthestTerm = i;
        }
    }
    nearestStartLocation = earliestPost;
    nearestEndLocation = docTracker[0].second;
    //Returns the end of the document that all the terms appear on
    return nearestStartLocation;
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


//TODO: UPDATE SEEK. REMEMBER TO UPDATE nearestTerm, farthestTerm, nearestStartLocation, nearestEndLocation too
Location ISRPhrase::seek(Location target){
    Vector<Location> termLocations;
    Location nearestStartLocationTracker = ULLONG_MAX;
    Location nearestEndLocationTracker = ULLONG_MAX;
    
    for (int i = 0; i < numOfTerms; ++i){
        while (terms[i]->CurInstance() < target && terms[i]->nextInstance() != 0){
            Location nextLocation = terms[i]->nextInstance();
            if (nextLocation >= target){
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
        if (i == numOfTerms - 1 && nearestStartLocationTracker == ULLONG_MAX){
            return 0;
        }
    }
    
    if (termLocations.empty()){
        return 0;
    }
    return termLocations[0];
}


