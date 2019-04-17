#include "Ranker.h"

void Ranker::seekIsrVector(Vector<Isr*>& isrVec, Location locationToSeek)
   {
   for(int i = 0; i < isrVec.size(); ++i)
      {
      isrVec[i]->SeekToLocation(locationToSeek);
      }
   }

void Ranker::seekDecoratedWordIsrs(Location locationToSeek, 
      DecoratedWordIsrs& wordIsrs)
   {
   seekIsrVector(wordIsrs.AnchorTextIsrs, locationToSeek);
   seekIsrVector(wordIsrs.BodyTextIsrs, locationToSeek);
   seekIsrVector(wordIsrs.TitleIsrs, locationToSeek);
   seekIsrVector(wordIsrs.UrlIsrs, locationToSeek);
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

void Ranker::getDocumentAttributes(Location matchLocation, IsrEndDoc* docIsr, 
      DocumentAttributes &docInfo)
   {
   Location endDocLocation = docIsr->SeekToLocation(matchLocation);
   if(endDocLocation == Isr::IsrSentinel)
      {
      fprintf(stderr, "isrWord has no doc end!")
      throw(1);
      }

   //TODO: change to working dummy implementation
   docInfo = IndexInterface::GetDocumentAttributes(endDocLocation);
   }

Location getDocStart(DocumentAttributes& docInfo, IsrEndDoc* docIsr)
   {
   return docIsr.GetCurrentLocation() - docInfo.DocumentLength;
   }

Vector<DocID> Ranker::Rank(Isr* rootIsr, DecoratedWordIsrs& wordIsrs,
      IsrEndDoc* docIsr)
   {
   Location matchLocation = rootIsr->NextInstance();
   while(matchLocation != Isr::IsrSentinel)
      {
      //get document info from current location
      Document document(matchLocation, docIsr);
      //move wordIsrs to beginning of doc
      Location docStartLocation = getDocStart(docInfo, docIsr);
      seekDecoratedWordIsrs(docStartLocation, wordIsrs);
      //score and rank
      document.ComputeScore(wordIsrs);
      updateTopRankedDocuments(document);
      matchingDocIndex = rootIsr->NextInstance();
      }
   }

Ranker::Document::Document(Location matchLocation, IsrEndDoc* docIsr)
   : docEndLocation(docIsr->GetCurrentLocation());
   {
   Location endDocLocation = docIsr->SeekToLocation(matchLocation);
   if(endDocLocation == Isr::IsrSentinel)
      {
      fprintf(stderr, "isrWord has no doc end!")
      throw(1);
      }

   //TODO: change to working dummy implementation
   docInfo = IndexInterface::GetDocumentAttributes(endDocLocation);
   }

unsigned short Ranker::Document::ComputeScore(DecoratedWordIsrs& wordIsrs)
   {
   decorationFeatures.AnchorTextFeatures.ComputeFeatures(wordIsrs.AnchorTextIsrs);
   decorationFeatures.BodyTextFeatures.ComputeFeatures(wordIsrs.BodyTextIsrs);
   decorationFeatures.TitleFeatures.ComputeFeatures(wordIsrs.TitleIsrs);
   decorationFeatures.UrlFeatures.ComputeFeatures(wordIsrs.UrlIsrs);

   //TODO: combine features with tuned weights
   return 0;
   }

Ranker::Document::Features::WordStatistics::WordStatistics(Isr* isrIn)
   : isr(isrIn) {}

void Ranker::Document::Features::ComputeFeatures(Vector<Isr*> wordIsrs)
   {
   //initialize Vector<WordStatistics>
   Vector<WordStatistics> wordsStatistics;
   for(int i = 0; i < wordIsrs.size(); ++i)
      {
      WordStatistics currentWordStatistic(wordIsrs[i]);
      wordsStatistics.push_back(currentWordStatistic);
      }
   
   bool allIsrsPastEnd = false;
   while(!allIsrsPastEnd)
      {
      //calculate term counts. TODO: expand to compute more features in this pass
      for(int i = 0; i < wordsStatistics.size(); ++i)
         {
         allIsrsPastEnd = true;
         if(wordsStatistics[i].IsPastEnd())
            {
            wordsStatistics[i].SeekNextInstance();
            allIsrsPastEnd = false;
            }
         }
      }
   }

void Ranker::Document::Features::WordStatistics::SeekNextInstance()
   {
   isr->NextInstance();
   if(!IsPastEnd())
      Count++;
   }

bool Ranker::Document::Features::WordStatistics::IsPastEnd()
   {
   return isr->GetCurrentLocation() >= docEndLocation;
   }