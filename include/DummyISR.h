//This header is for the sake of testing ranker feaeture extraction without
//relying on real documents
#pragma once
#ifndef DUMMYISR_H
#define DUMMYISR_H

#include "vector.h"
#include "IndexInterface.h"

namespace IsrGlobals
   {
   const Location IsrSentinel = 0;
   //TODO: CHange to 1
   const Location IndexStart = 1;
   }

class Isr
   {
   public:
   //These need to implemented in real ISR
   Isr(Vector<Location> matchesIn);
   Location NextInstance();
   Location SeekToLocation(Location docStart);
   Location GetCurrentLocation();
   Location SeekDocStart(Location docStart);

   protected:
   //TO JASON AND BRADLEY:
   //curLocation actually does need to be implemented 
   Location curLocation;
   //Other private members are just for the sake of my own dummyIsr implementation
   Vector<Location> matches;
   unsigned curInd;
   bool hasNextInstance();
   };

class IsrEndDoc : public Isr
   {
   public:
      IsrEndDoc(Vector<Location> matchesIn);
      DocumentAttributes GetDocInfo();
   };

#endif