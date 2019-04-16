#include "DummyISR.h"

IsrDummy::IsrDummy(vector<Location> matchesIn)
    : matches(matchesIn), curInd(0) {}

Location IsrDummy::NextInstance()
   {
   return matches[++curInd];
   }

Location IsrDummy::SeekDocStart(Location docStart)
   {
   Location closestLocation = ULLONG_MAX;
   while(HasNextInstance() && closestLocation < docStart)
      closestLocation = NextInstance();      
   return closestLocation;
   }

bool IsrDummy::HasNextInstance()
   {
   return curInd < matches.size() - 2;
   }