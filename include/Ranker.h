#include "DummyIsr.h"
#include "vector.h"

typedef unsigned DocID;

//all Isrs should be flattened and grouped by decoration
//e.g. for query "Dennis AND (Dan OR Joe)"
//give me [Dennis Dan Joe], [#Dennis #Dan #Joe], [@Dennis @Dan @Joe], [/Dennis /Dan /Joe]
//where each of the above terms (e.g. Dennis, #Dennis, /Joe, etc.) are the
//WordIsr* for that term 
//Be sure to maintain the original ordering of the flattened query
struct DecoratedWordIsrs
   {
   Vector<Isr*> AnchorTextIsrs;
   Vector<Isr*> BodyTextIsrs;
   Vector<Isr*> TitleIsrs;
   Vector<Isr*> UrlIsrs;
   };

class Ranker
   {
   public:
      Ranker();
      //RETURNS: Document IDs ranked from most relevant to least relevant
      Vector<DocID> Rank(Isr* rootIsr, DecoratedWordIsrs& wordIsrs, IsrDocDummy* docIsr);
   
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
            //Seeks docIsr to endDoc location after matchLocation
            Document(Location matchLocation, IsrEndDoc* docIsr);
            unsigned short Score;
            //REQUIRES: wordIsrs are seeked to beginning of this document
            unsigned short ComputeScore(DecoratedWordIsrs& wordIsrs);
            DocID GetDocID();
         private:
            class Features
               {
               //public variables are the features directly used in ranking{
               public: 
                  void ComputeFeatures(Vector<Isr*> wordIsrs);
                  unsigned short TfIdf;
                  unsigned short StaticRank;
                  unsigned short NormalizedSumOfStreamLength;
                  unsigned short NumAnchorTextReferences;
               private:
                  class WordStatistics
                     {
                     public:
                        WordStatistics(Isr* isrIn);
                        void SeekNextInstance();
                        bool IsPastEnd();
                        unsigned short Count;
                     private:
                        Isr* isr;
                     };
               };
                  
            struct DecorationFeatures
               {
               Features AnchorTextFeatures;
               Features BodyTextFeatures;
               Features TitleFeatures;
               Features UrlFeatures;
               };
            
            DocumentAttributes docInfo;
            DecorationFeatures decorationFeatures;
            Location docEndLocation;
         };

      Vector<RankedDocument> topRankedDocuments;

      void seekDecoratedWordIsrs(Location locationToSeek, 
            DecoratedWordIsrs& wordIsrs);
      void seekIsrVector(Vector<Isr*> &isrVec, Location locationToSeek);
      void updateTopRankedDocuments(Document& document);
      void clearTopRankedDocments();
      DocID getDocumentID(PostingListIndex* docIndex);
   };
