#include "ISR.hpp"
#include "vector.h"

typedef unsigned DocID;
class Ranker
   {
public:
   Ranker();
   Vector<DocID> rank(ISR &isr);
   
private:
   class RankedDocument
      {
   private:
      DocID ID;
      unsigned short Score;
      };

   class Document
      {
   public:
      DocID ID;
      unsigned short Score;
      void ComputeScore();
   private:
      unsigned short Tf_idf;
      unsigned short static_rank;
      unsigned short num_anchor_text_references;
      };
   Vector<RankedDocument> TopRankedDocuments;
   };

//TODO: Add rest of features, add SortedInsert method to Vector class 