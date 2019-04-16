//This header is for the sake of testing ranker feaeture extraction without
//relying on real documents
#include <vector>
#include <climits>
using namespace std;

typedef unsigned long long Location;


class IsrDummy
   {
   public:
      IsrDummy(vector<Location> matchesIn);
      Location NextInstance();
      Location SeekDocStart(Location docStart);
      bool HasNextInstance();
   private:
      vector<Location> matches;
      unsigned curInd;
   };