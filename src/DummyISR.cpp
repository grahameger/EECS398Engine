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

Location Isr::SeekToLocation(Location location)
   {
   Location closestLocation = IsrGlobals::IsrSentinel;
   curInd = 0;
   if(matches.size() > 0)
      {
      closestLocation = matches[0]; 
      }

   while(curInd < matches.size() - 1 && matches[curInd] < location)
      {
      curInd++;
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
   curLocation = closestLocation;
   return closestLocation;
   }

bool Isr::hasNextInstance()
   {
   return curInd < matches.size() - 1;
   }

Location Isr::GetCurrentLocation()
   {
   return curLocation;
   }

DocumentAttributes IsrEndDoc::GetDocInfo()
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