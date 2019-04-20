#include "vector.h"
#include "Ranker.h"
#include <vector>

void LoadVector(Vector<Location>& dst, std::vector<Location>& src)
   {
   for(size_t i = 0; i < src.size(); ++i)
      {
      dst.push_back(src[i]);
      } 
   }

int main()
   {
   std::vector<Location> docEndSrc = {10, 20, 30};
   std::vector<Location> dennisAnchorSrc = {2, 4, 5, 12, 17};
   std::vector<Location> dennisBodySrc = {3, 13, 24};
   std::vector<Location> dennisTitleSrc = {21};
   std::vector<Location> dennisUrlSrc = {7, 14, 16, 23};
   //std::vector<Location> liSrc = {14, 15, 18};

   Vector<Location> docEndLocations;
   Vector<Location> dennisAnchorLocations;
   //Vector<Location> liLocations;

   LoadVector(dennisAnchorLocations, dennisAnchorSrc);
   LoadVector(docEndLocations, docEndSrc);
   //LoadVector(liLocations, liSrc);

   Isr dennis(dennisAnchorLocations);
   //Isr li(liLocations);
   IsrEndDoc docEnd(docEndLocations);




   


   }