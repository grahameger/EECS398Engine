#include "Ranker.h"
//PostingListIndex to be created
class PostingListIndex;

void Ranker::seekISRWordVector(Vector<ISRWord*>& isrVec, Location locationToSeek)
   {
   for(int i = 0; i < isrVec.size(); ++i)
      {
      isrVec[i]->seekDocStart(locationToSeek);
      }
   }

void Ranker::seekDecoratedWordISRs(Location locationToSeek, 
      DecoratedWordISRs& wordISRs)
   {
   seekISRWordVector(wordISRs.AnchorTextISRs, locationToSeek);
   seekISRWordVector(wordISRs.BodyTextISRs, locationToSeek);
   seekISRWordVector(wordISRs.TitleISRs, locationToSeek);
   seekISRWordVector(wordISRs.UrlISRs, locationToSeek);
   }

void Ranker::updateTopRankedDocuments(Document& document)
   {
   //todo add sortedInsert method to Vector class 
   }

DocID Ranker::getDocumentID(PostingListIndex* docIndex)
   {
   //TODO implement
   return 0;
   }

Vector<DocID> Ranker::Rank(ISR* rootISR, DecoratedWordISRs& wordISRs)
   {
   PostingListIndex* matchingDocIndex = rootISR->nextInstance();
   while(matchingDocIndex)
      {
      Document document(getDocumentID(matchingDocIndex));
      Location docStart = rootISR->getDocStartLocation();
      seekDecoratedWordISRs(docStart, wordISRs);
      document.ComputeScore(wordISRs);
      updateTopRankedDocuments(document);
      matchingDocIndex = rootISR->nextInstance();
      }
   }

Ranker::Document::Document(DocID idIn)
   : ID(idIn) {}

unsigned short Ranker::Document::ComputeScore(DecoratedWordISRs& wordISRs)
   {
   decorationFeatures.AnchorTextFeatures.ComputeFeatures(wordISRs.AnchorTextISRs);
   decorationFeatures.BodyTextFeatures.ComputeFeatures(wordISRs.BodyTextISRs);
   decorationFeatures.TitleFeatures.ComputeFeatures(wordISRs.TitleISRs);
   decorationFeatures.UrlFeatures.ComputeFeatures(wordISRs.UrlISRs);

   //TODO: combine features with tuned weights
   return 0;
   }

Ranker::Document::Features::WordStatistics::WordStatistics(ISRWord* isrIn)
   : isr(isrIn) {}

void Ranker::Document::Features::ComputeFeatures(Vector<ISRWord*> wordISRs)
   {
   //initialize Vector<WordStatistics>
   Vector<WordStatistics> wordsStatistics;
   for(int i = 0; i < wordISRs.size(); ++i)
      {
      WordStatistics currentWordStatistic(wordISRs[i]);
      wordsStatistics.push_back(currentWordStatistic);
      }
   
   bool allISRsPastEnd = false;
   while(!allISRsPastEnd)
      {
      //calculate term counts. TODO: expand to compute more features in this pass
      for(int i = 0; i < wordsStatistics.size(); ++i)
         {
         allISRsPastEnd = true;
         if(wordsStatistics[i].IsPastEnd())
            {
            wordsStatistics[i].SeekNextInstance();
            allISRsPastEnd = false;
            }
         }
      }
   }

void Ranker::Document::Features::WordStatistics::SeekNextInstance()
   {
   isr->nextInstance();
   if(!IsPastEnd())
      Count++;
   }