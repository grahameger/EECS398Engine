#include "ISR.hpp"
#include "vector.h"

typedef unsigned DocID;

//all ISRs should be flattened and grouped by decoration
//e.g. for query "Dennis AND (Dan OR Joe)"
//give me [Dennis Dan Joe], [#Dennis #Dan #Joe], [@Dennis @Dan @Joe], [/Dennis /Dan /Joe]
//where each of the above terms (e.g. Dennis, #Dennis, /Joe, etc.) are the
//WordISR* for that term 
//Be sure to maintain the original ordering of the flattened query
struct DecoratedWordISRs
   {
   Vector<ISRWord*> AnchorTextISRs;
   Vector<ISRWord*> BodyTextISRs;
   Vector<ISRWord*> TitleISRs;
   Vector<ISRWord*> UrlISRs;
   };

class Ranker
   {
   public:
      Ranker();
      //RETURNS: Document IDs ranked from most relevant to least relevant
      Vector<DocID> Rank(ISR *rootISR, DecoratedWordISRs& wordISRs);
   
   private:
      class RankedDocument
         {
         private:
            DocID id;
            unsigned short score;
         };

      class Document
         {
         public:
            Document(DocID idIn);
            DocID ID;
            unsigned short Score;
            //REQUIRES: wordISRs are seeked to beginning of this document
            unsigned short ComputeScore(DecoratedWordISRs& wordISRs);
         private:
            class Features
               {
               //public variables are the features directly used in ranking{
               public: 
                  void ComputeFeatures(Vector<ISRWord*> wordISRs);
                  unsigned short TfIdf;
                  unsigned short StaticRank;
                  unsigned short NormalizedSumOfStreamLength;
                  unsigned short NumAnchorTextReferences;
               private:
                  class WordStatistics
                     {
                     public:
                        WordStatistics(ISRWord* isrIn);
                        void SeekNextInstance();
                        bool IsPastEnd();
                        unsigned short Count;
                     private:
                        ISRWord* isr;
                     };
               };
                  
            struct DecorationFeatures
               {
               Features AnchorTextFeatures;
               Features BodyTextFeatures;
               Features TitleFeatures;
               Features UrlFeatures;
               };

            DecorationFeatures decorationFeatures;
         };

      Vector<RankedDocument> topRankedDocuments;

      void seekDecoratedWordISRs(Location locationToSeek, 
            DecoratedWordISRs& wordISRs);
      void seekISRWordVector(Vector<ISRWord*> &isrVec, Location locationToSeek);
      void updateTopRankedDocuments(Document& document);
      void clearTopRankedDocments();
      DocID getDocumentID(PostingListIndex* docIndex);
   };