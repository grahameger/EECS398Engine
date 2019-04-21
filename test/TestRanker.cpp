#include "vector.h"
#include "Ranker.h"
#include <vector>
#include <stdio.h>

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
   //todo make it so that it will always take me to diff document
   std::vector<Location> rootIsrSrc = {2, 12, 21};
   //std::vector<Location> liSrc = {14, 15, 18};

   Vector<Location> docEndLocations;
   Vector<Location> dennisAnchorLocations;
   Vector<Location> dennisBodyLocations;
   Vector<Location> dennisTitleLocations;
   Vector<Location> dennisUrlLocations;
   Vector<Location> rootIsrLocations;
   //Vector<Location> liLocations;

   LoadVector(dennisAnchorLocations, dennisAnchorSrc);
   LoadVector(dennisBodyLocations, dennisBodySrc);
   LoadVector(dennisTitleLocations, dennisTitleSrc);
   LoadVector(dennisUrlLocations, dennisUrlSrc);
   LoadVector(docEndLocations, docEndSrc);
   LoadVector(rootIsrLocations, rootIsrSrc);
   //LoadVector(liLocations, liSrc);

   Isr dennisAnchor(dennisAnchorLocations);
   Isr dennisBody(dennisBodyLocations);
   Isr dennisTitle(dennisTitleLocations);
   Isr dennisUrl(dennisUrlLocations);
   Isr rootIsr(rootIsrLocations);
   //Isr li(liLocations);
   IsrEndDoc docEnd(docEndLocations);

   Vector<Isr*> anchorTextIsrVec;
   anchorTextIsrVec.push_back(&dennisAnchor);
   
   Vector<Isr*> bodyTextIsrVec;
   bodyTextIsrVec.push_back(&dennisBody);

   Vector<Isr*> titleIsrVec;
   titleIsrVec.push_back(&dennisTitle);

   Vector<Isr*> urlIsrVec;
   urlIsrVec.push_back(&dennisUrl);

   DecoratedWordIsrs decWords = {anchorTextIsrVec, bodyTextIsrVec, titleIsrVec, urlIsrVec};

   Ranker ranker;
   Vector<ScoredDocument> topTen = ranker.Rank(&rootIsr, decWords, &docEnd);
   
   for(int i = 0; i < topTen.size(); ++i)
      {
      printf("%d", topTen[i].id);
      printf("\n");
      printf("%d", topTen[i].score); 
      printf("\n");
      }
   }