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
   std::vector<Location> dennisAnchorSrc = {12, 17};
   std::vector<Location> dennisBodySrc = {3, 13, 24};
   std::vector<Location> dennisTitleSrc {1, 2};
   std::vector<Location> dennisUrlSrc = {7, 14, 16, 23};
   //todo make it so that it will always take me to diff document
   std::vector<Location> rootIsrSrc = {1, 2, 14, 16, 23, 24};
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
   Vector<Location> liTitleLocations2 = {4, 5, 18, 22};
   //LoadVector(liLocations, liSrc);

   Isr dennisAnchor(dennisAnchorLocations);
   dennisAnchor.AddWord("Dennis");
   dennisAnchor.SetImportance(5);
   Isr dennisBody(dennisBodyLocations);
   dennisBody.SetImportance(9);
   dennisBody.AddWord("Dennis");
   Isr dennisTitle(dennisTitleLocations);
   dennisTitle.SetImportance(11);
   dennisTitle.AddWord("Dennis");
   Isr dennisUrl(dennisUrlLocations);
   dennisUrl.AddWord("Dennis");
   dennisTitle.SetImportance(3);
   Isr rootIsr(rootIsrLocations);
   Isr liTitle2(liTitleLocations2);
   dennisTitle2.AddWord("Li");
   dennisTitle.SetImportance(3);
   //Isr li(liLocations);
   IsrEndDoc docEnd(docEndLocations);

   Vector<Isr*> anchorTextIsrVec;
   anchorTextIsrVec.push_back(&dennisAnchor);
   
   Vector<Isr*> bodyTextIsrVec;
   bodyTextIsrVec.push_back(&dennisBody);

   Vector<Isr*> titleIsrVec;
   titleIsrVec.push_back(&dennisTitle);
   titleIsrVec.push_back(&liTitle2);

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