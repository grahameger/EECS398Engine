//This header is for the sake of testing ranker feaeture extraction without
//relying on real documents
#include "Vector.h"
#include <climits>

typedef unsigned long long Location;
namespace Isr
   {
   const Location IsrSentinel;
   }

class Isr
{
public:
   //These need to implemented in real ISR
   Isr(Vector<Location> matchesIn);
   Location NextInstance();
   Location SeekToLocation(Location docStart);
   Location GetCurrentLocation();
private:
   //TO JASON AND BRADLEY:
   //curLocation actually does need to be implemented 
   Location curLocation;
   //Other private members are just for the sake of my own dummyIsr implementation
   Vector<Location> matches;
   unsigned curInd;
   bool HasNextInstance();
};

//I won't ever use NextInstance() for EndDoc ISR,
//so it can do whatever (e.g. take me to next end doc
//irrespective of query)
class IsrEndDoc : public Isr
{
public:
private:
};