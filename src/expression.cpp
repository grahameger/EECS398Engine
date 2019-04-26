//
//  constraint_solver.cpp
//
//
//  Created by Bradley on 4/3/19.
//
//
#include "expression.h"
#include <climits>

Location Isr::SeekToLocation(Location target) {
    if(GetCurrentLocation() == IsrGlobals::IsrSentinel
            || GetCurrentLocation() > target)
        ResetToStart();
    
    Location closestLocation = IsrGlobals::IsrSentinel;
    while(GetCurrentLocation() != IsrGlobals::IsrSentinel
            && GetCurrentLocation() < target)
        NextInstance();
    return GetCurrentLocation();
}

Location Isr::GetCurrentLocation() {
    return currentLocation;
}

void Isr::addTerm(Isr* isr) {
   terms.push_back(isr);
}

Isr::~Isr() {
    for(size_t i = 0; i < terms.size(); ++i) {
        delete terms[i];
        terms[i] = nullptr;
    }
}

Isr::Isr()
   : currentLocation(IsrGlobals::IsrSentinel) {}

void IsrEndDoc::AddMatches(Vector<Location>& matchesIn)
{
    matches = matchesIn;
}

IsrWord::IsrWord(String& wordIn)
: word(wordIn), curInd(0)
{
    String quick = "quick";
    String brown = "brown";
    String fox = "fox";
    String empty = "";
    if(word.Compare(quick))
    {
        matches.push_back(4);
        matches.push_back(5);
        matches.push_back(13);
        matches.push_back(22);
    }
    else if(word.Compare(brown))
    {
        matches.push_back(3);
        matches.push_back(7);
    }
    else if(word.Compare(fox))
    {
        matches.push_back(2);
        matches.push_back(14);
        matches.push_back(23);
    }
    else if(word.Compare(empty))
    {
        matches.push_back(10);
        matches.push_back(20);
        matches.push_back(30);
        matches.push_back(40);
    }
    else
    {
        printf("wrong word!!!!!!!!");
        return;
    }
    
    //populate current instance
    if(matches.empty())
        currentLocation = 0;
    else
        currentLocation = matches[0];
}

Location IsrWord::NextInstance()
{
   if(hasNextInstance())
      currentLocation = matches[++curInd];
   else
      currentLocation = IsrGlobals::IsrSentinel;
   return currentLocation;
}

Location IsrWord::ResetToStart()
{
    curInd = 0;
    if(matches.empty())
       {
       currentLocation = IsrGlobals::IsrSentinel;
       return IsrGlobals::IsrSentinel;
       }
    currentLocation = matches[0];
    return matches[0];
}

void IsrWord::SetLocations (Vector<Location>& matchesIn)
{
   matches = matchesIn;
}

Location IsrWord::SeekToLocation(Location location)
{
   Location closestLocation = IsrGlobals::IsrSentinel;
   curInd = 0;
   if(matches.size() > 0)
   {
      closestLocation = matches[0];
   }
   
   while(curInd < matches.size() - 1 && matches[curInd] < closestLocation)
   {
      ++curInd;
      closestLocation = matches[curInd];
   }
   
   if(curInd == matches.size() - 1)
   {
      if(matches[curInd] > location)
      {
         closestLocation = matches[curInd];
      }
      else
         closestLocation = IsrGlobals::IsrSentinel;
   }
   currentLocation = closestLocation;
   return closestLocation;
}

bool IsrWord::hasNextInstance()
{
   return curInd < matches.size() - 1;
}

Location IsrWord::GetCurrentLocation()
{
   return currentLocation;
}

void IsrEndDoc::saveDocInfo() {
    if(docInfo)
        return;
    
    docInfo = new DocumentAttributes;
    docInfo->DocID = curInd;
    if(curInd == 0)
    {
        docInfo->DocumentLength = matches[curInd] - 1;
    }
    else
    {
        docInfo->DocumentLength = matches[curInd] - matches[curInd - 1];
    }
}

IsrEndDoc::IsrEndDoc()
    : currentLocation(IsrGlobals::IsrSentinel) {}

IsrEndDoc::~IsrEndDoc() {
    delete docInfo;
    docInfo = nullptr;
}

DocumentAttributes* IsrEndDoc::GetDocInfo()
{
    saveDocInfo();
    return docInfo;
}

Location IsrEndDoc::GetDocLength()
{
    saveDocInfo();
    return docInfo->DocumentLength;
}

DocumentAttributes IsrWord::GetDocInfo()
{
    DocumentAttributes docInfo;
    docInfo.DocID = curInd;
    if(curInd == 0)
    {
        docInfo.DocumentLength = matches[curInd] - 1;
    }
    else
    {
        docInfo.DocumentLength = matches[curInd] - matches[curInd - 1];
    }
    return docInfo;
}


void IsrWord::AddWord(String &wordIn)
{
   word = wordIn;
}

void IsrWord::SetImportance(unsigned importanceIn)
{
   importance = importanceIn;
}

//////////
//OR ISR//
//////////
IsrOr::IsrOr(Vector<Isr*>& phrasesToInsert) 
{
   for (int i = 0; i < phrasesToInsert.size(); ++i) {
      terms.push_back(phrasesToInsert[i]);
   }
}

Location SeekToNextDocument(Isr* isr, Location docEndLocation) {
   while(isr->GetCurrentLocation() < docEndLocation && 
         isr->GetCurrentLocation() != IsrGlobals::IsrSentinel)
      isr->NextInstance();
   return isr->GetCurrentLocation();
}

void MoveDocEndToCurrentDocument(IsrEndDoc* docIsr, Isr* curIsr) {
   docIsr->SeekToLocation(curIsr->GetCurrentLocation());
}

DocumentLocation GetDocumentLocation(IsrEndDoc* isr) {
    DocumentLocation docLocation;
    docLocation.docEnd = isr->GetCurrentLocation();
    docLocation.docStart = docLocation.docEnd - isr->GetDocLength();
    return docLocation;
}

//returns isrsentinel if error, otherwise any one of the terms
//Does not change curlocation!
Location Isr::MoveAllTermsOffCurrentDocument() {
    if(GetCurrentLocation() == IsrGlobals::IsrSentinel)
      return IsrGlobals::IsrSentinel;

    if(terms.empty()) {
       return IsrGlobals::IsrSentinel;
    }

    IsrEndDoc* endDocIsr = new IsrEndDoc;
    MoveDocEndToCurrentDocument(endDocIsr, this);
    Location curDocEnd = endDocIsr->GetCurrentLocation();

    bool allIsrsPastPageEnd = false;
    while(!allIsrsPastPageEnd) {
        allIsrsPastPageEnd = true;
        for(size_t i = 0; i < terms.size(); ++i) {
            if(terms[i]->GetCurrentLocation() == IsrGlobals::IsrSentinel) {
                return IsrGlobals::IsrSentinel;
            }
            else if(terms[i]->GetCurrentLocation() < curDocEnd) {
                terms[i]->NextInstance();
                allIsrsPastPageEnd = false;
            }
        }
    }
    delete endDocIsr;
    endDocIsr = nullptr;
    return terms[0]->GetCurrentLocation();
}

Location IsrOr::NextInstance() {
    if(GetCurrentLocation() == IsrGlobals::IsrSentinel) {
        currentLocation = IsrGlobals::IsrSentinel;
        return currentLocation;
    }
    if(GetCurrentLocation() == IsrGlobals::IsrSentinel) {
        currentLocation = IsrGlobals::IsrSentinel;
        return currentLocation;
    }
    //else return isr with min location
    Isr* minIsr = terms[0];
    for(size_t i = 1; i < terms.size(); ++i) {
        if(terms[i]->GetCurrentLocation() < minIsr->GetCurrentLocation())
            minIsr = terms[i];
    }
    
    currentLocation = minIsr->GetCurrentLocation();
    return currentLocation;
}

Location IsrOr::ResetToStart() {
   if(terms.empty()) {
      currentLocation = IsrGlobals::IsrSentinel;
      return currentLocation;
   }
   for(size_t i = 0; i < terms.size(); ++i) {
      terms[i]->ResetToStart();
   }

   Isr* minLocationIsr = terms[0];
   for(size_t i = 0; i < terms.size(); ++i) {
      if(terms[i]->GetCurrentLocation() == IsrGlobals::IsrSentinel) {
         currentLocation = IsrGlobals::IsrSentinel;
         return currentLocation;
      }
      if(terms[i] < minLocationIsr)
         minLocationIsr = terms[i];
   }
   currentLocation = minLocationIsr->GetCurrentLocation();
   return currentLocation; 
}
    
/////////////////
//End of OR ISR//
/////////////////

///////////
//AND ISR//
///////////

IsrAnd::IsrAnd(Vector<Isr*>& phrasesToInsert) {
   for (int i = 0; i < phrasesToInsert.size(); ++i) {
      terms.push_back(phrasesToInsert[i]);
   }
}

bool IsOnDoc(DocumentLocation& docLocation, Isr* isr) {
   if(isr->GetCurrentLocation() == IsrGlobals::IsrSentinel)
      return false;
   else
      return isr->GetCurrentLocation() > docLocation.docStart
         && isr->GetCurrentLocation() < docLocation.docEnd;
}

Location IsrAnd::moveTermsToSameDocument(DocumentLocation& docLocation) {
   for(size_t i = 0; i < terms.size(); ++i)
      terms[i]->SeekToLocation(docLocation.docStart);
}

Location IsrAnd::NextInstance() {
   if(GetCurrentLocation() == IsrGlobals::IsrSentinel) {
      currentLocation = IsrGlobals::IsrSentinel;
      return currentLocation;
   }

   Isr* maxLocationIsr = terms[0];
   bool allIsrsOnSameDoc = false;
   while(!allIsrsOnSameDoc) {
      allIsrsOnSameDoc = true;
      //find max location isr
      for(size_t i = 0; i < terms.size(); ++i) {
         if(terms[i]->GetCurrentLocation() == IsrGlobals::IsrSentinel) {
            currentLocation = IsrGlobals::IsrSentinel;
            return currentLocation;
         }
         if(terms[i]->GetCurrentLocation() > maxLocationIsr->GetCurrentLocation())
            maxLocationIsr = terms[i];
      }
      //attempt to move all isrs to same document as maxLocIsr
      IsrEndDoc* endDocIsr = new IsrEndDoc;
      MoveDocEndToCurrentDocument(endDocIsr, this);
      DocumentLocation docLocation = GetDocumentLocation(endDocIsr);

      for(size_t i = 0; i < terms.size(); ++i) {
         if(!IsOnDoc(docLocation, terms[i])) {
            allIsrsOnSameDoc = false;
            terms[i]->SeekToLocation(docLocation.docStart);
         }
      }
   }

   currentLocation = terms[0]->GetCurrentLocation();
   return currentLocation;
}

Location IsrAnd::ResetToStart() {
   if(terms.empty()) {
      currentLocation = IsrGlobals::IsrSentinel;
      return currentLocation;
   }
   for(size_t i = 0; i < terms.size(); ++i) {
      terms[i]->ResetToStart();
   }

   currentLocation = IsrGlobals::IndexStart;
   return NextInstance();
}

Location IsrPhrase::NextInstance() {
   if(GetCurrentLocation() == IsrGlobals::IsrSentinel) {
      currentLocation = IsrGlobals::IsrSentinel;
      return currentLocation;
   }

   Isr* maxLocationIsr = terms[0];
   size_t maxLocationIsrInd = 0;
   bool hasExactMatch = false;
   while(!hasExactMatch) {
      hasExactMatch = true;
      //find max location isr
      for(size_t i = 0; i < terms.size(); ++i) {
         if(terms[i]->GetCurrentLocation() == IsrGlobals::IsrSentinel) {
            currentLocation = IsrGlobals::IsrSentinel;
            return currentLocation;
         }
         if(terms[i]->GetCurrentLocation() > maxLocationIsr->GetCurrentLocation()) {
            maxLocationIsr = terms[i];
            maxLocationIsrInd = i;
         }
      }
      //attempt to move to correct position
      for(size_t i = 0; i < terms.size(); ++i) {
          //check
         if(terms[i]->GetCurrentLocation() == IsrGlobals::IsrSentinel) {
            currentLocation = IsrGlobals::IsrSentinel;
            return currentLocation;
         }
         
         int correctOffset = i - maxLocationIsr->GetCurrentLocation();
         Location correctLocation = 
               maxLocationIsr->GetCurrentLocation() + correctOffset;
         terms[i]->SeekToLocation(correctLocation);
         //check
         if(terms[i]->GetCurrentLocation() == IsrGlobals::IsrSentinel) {
            currentLocation = IsrGlobals::IsrSentinel;
            return currentLocation;
         }

         if(terms[i]->GetCurrentLocation() != correctLocation) {
            hasExactMatch = false;
            break;
         }
      }
   }

   currentLocation = terms[0]->GetCurrentLocation();
   return currentLocation;
}

Location IsrPhrase::ResetToStart() {
   if(terms.empty()) {
      currentLocation = IsrGlobals::IsrSentinel;
      return currentLocation;
   }
   for(size_t i = 0; i < terms.size(); ++i) {
      terms[i]->ResetToStart();
   }

   currentLocation = IsrGlobals::IndexStart;
   return NextInstance();
}