#include "DummyISR.h"

Isr::Isr(Vector<Location> matchesIn)
    : matches(matchesIn), curInd(0) {}

IsrEndDoc::IsrEndDoc(Vector<Location> matchesIn)
    : Isr(matchesIn) {}

Location Isr::NextInstance()
   {
   if(hasNextInstance())
      curLocation = matches[++curInd];
   else
      curLocation = IsrGlobals::IsrSentinel;
   return curLocation;
   }

Location Isr::SeekDocStart(Location docStart)
   {
   Location closestLocation = IsrGlobals::IsrSentinel;
   while(hasNextInstance() && closestLocation < docStart)
      closestLocation = NextInstance();      
   return closestLocation;
   }

bool Isr::hasNextInstance()
   {
   return curInd < matches.size() - 2;
   }

Location Isr::GetCurrentLocation()
   {
   return curInd;
   }

DocumentAttributes IsrEndDoc::GetDocInfo()
   {
   DocumentAttributes docInfo;
   docInfo.DocID = curInd;
   if(curInd == 0)
      {
      docInfo.DocumentLength = matches[curInd];
      }
   else
      {
      docInfo.DocumentLength = matches[curInd] - matches[curInd - 1];
      }
   return docInfo;
   }