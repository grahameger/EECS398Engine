#pragma once
#include "DummyISR.h"
#include "vector.h"
#include "RankerParams.h"
#include "IndexInterface.h"
#include <stdio.h>

typedef unsigned DocID;

struct DecoratedWordIsrs
      {
      Vector<Isr*> AnchorTextIsrs;
      Vector<Isr*> BodyTextIsrs;
      Vector<Isr*> TitleIsrs;
      Vector<Isr*> UrlIsrs;
      };

struct ScoredDocument
      {
      DocID id;
      unsigned score;
      };

enum TextType {anchor, body, title, url};

//all Isrs should be flattened and grouped by decoration
//e.g. for query "Dennis AND (Dan OR Joe)"
//give me [Dennis Dan Joe], [#Dennis #Dan #Joe], [@Dennis @Dan @Joe], [/Dennis /Dan /Joe]
//where each of the above terms (e.g. Dennis, #Dennis, /Joe, etc.) are the
//WordIsr* for that term 
//Be sure to maintain the original ordering of the flattened query
class Ranker
   {
   public:
      Ranker();
      //RETURNS: Document IDs ranked from most relevant to least relevant
      Vector<ScoredDocument> Rank(Isr* rootIsr, DecoratedWordIsrs& wordIsrs, IsrEndDoc* docIsr);
   
   private:
      //sorting with this ranking function will give you documents
      //from best to worst
      class ScoredDocumentCompare
            {
            public:
            bool operator() (ScoredDocument doc1, ScoredDocument doc2);
            };

      class Document
         {
         public:
            //Seeks docIsr to endDoc location after matchLocation
            Document(Location matchLocation, IsrEndDoc* docIsr);
            unsigned GetScore();
            //REQUIRES: wordIsrs are seeked to beginning of this document
            unsigned ComputeScore(DecoratedWordIsrs& wordIsrs);
            DocID GetDocID();
            DocumentAttributes GetDocInfo();
         private:
            class Features
               {
               //public variables are the features directly used in ranking{
               public: 
                  unsigned TfIdf;
                  unsigned StaticRank;
                  unsigned NormalizedSumOfStreamLength;
                  unsigned NumAnchorTextReferences;
                  unsigned TotalWordFrequency;
                  void SetFeatureType(TextType textTypeIn);
                  void SetCurrentDocument(Document* docIn);
                  unsigned ComputeScore(Vector<Isr*>& isr);
                  class WordStatistics
                     {
                     public:
                        WordStatistics(Isr* isrIn, Document* curDocumentIn);
                        WordStatistics();
                        void SeekNextInstance();
                        bool IsPastEnd();
                        unsigned Count;
                     private:
                        Isr* isr;
                        Document* curDocument;
                     };

                  void computeFeatures(Vector<Isr*> wordIsrs);
               
               private:
                  Document* curDocument;
                  TextType textType;
                  unsigned textTypeWeight;
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
            unsigned score;
         };

      Vector<ScoredDocument> topRankedDocuments;
      unsigned numDocsToDisplay; 

      void seekDecoratedWordIsrs(Location locationToSeek, DecoratedWordIsrs& wordIsrs);
      void seekIsrVector(Vector<Isr*> &isrVec, Location locationToSeek);
      void updateTopRankedDocuments(Document& document);
      void clearTopRankedDocments();
      void resetIsrVec(Vector<Isr*> &isrVec);
      void resetAllIsr(Isr* rootIsr, DecoratedWordIsrs& wordIsrs, IsrEndDoc* docIsr);
   };