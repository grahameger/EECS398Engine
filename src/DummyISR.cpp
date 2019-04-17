#include "DummyISR.h"

IsrDummy::IsrDummy(vector<Location> matchesIn)
    : matches(matchesIn), curInd(0), IsrSentinel(ULLONG_MAX) {}

Location IsrDummy::NextInstance()
   {
   if(HasNextInstance())
      curLocation = matches[++curInd];
   else
      curLocation = ULLONG_MAX;
   return curLocation;
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

Location IsrDummy::GetCurrentLocation()
   {
   return curInd;
   }