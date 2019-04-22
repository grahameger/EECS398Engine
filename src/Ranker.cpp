#include "Ranker.h"

void Ranker::seekIsrVector(Vector<Isr*>& isrVec, Location locationToSeek)
   {
   for(size_t i = 0; i < isrVec.size(); ++i)
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
   ScoredDocument newDoc = {document.GetDocID(), document.GetScore()};
   ScoredDocumentCompare compare;
   topRankedDocuments.insertSortedMaxSize(newDoc, compare, numDocsToDisplay);
   }

Location Ranker::getDocStart(unsigned docLength, IsrEndDoc* docIsr)
   {
   return docIsr->GetCurrentLocation() - docLength;
   }

void Ranker::resetIsr(Isr* isr)
   {
   isr->SeekToLocation(IsrGlobals::IndexStart);
   }

void Ranker::resetIsrVec(Vector<Isr*> &isrVec)
   {
   for(size_t i = 0; i < isrVec.size(); ++i)
      {
      resetIsr(isrVec[i]);
      }
   }

DocID Ranker::Document::GetDocID()
   {
   return docInfo.DocID; 
   }

void Ranker::resetAllIsr(Isr* rootIsr, DecoratedWordIsrs& wordIsrs, 
      IsrEndDoc* docIsr)
   {
   resetIsr(rootIsr);
   resetIsr(docIsr);
   resetIsrVec(wordIsrs.AnchorTextIsrs);
   resetIsrVec(wordIsrs.BodyTextIsrs);
   resetIsrVec(wordIsrs.TitleIsrs);
   resetIsrVec(wordIsrs.UrlIsrs);
   }

Location Ranker::getNextDocumentLocation(Isr* rootIsr, Location docEndLocation)
   {
   while(rootIsr->GetCurrentLocation() < docEndLocation && 
         rootIsr->GetCurrentLocation() != IsrGlobals::IsrSentinel)
      rootIsr->NextInstance();
   return rootIsr->GetCurrentLocation();
   }

Vector<ScoredDocument> Ranker::Rank(Isr* rootIsr, DecoratedWordIsrs& wordIsrs,
      IsrEndDoc* docIsr)
   {
   resetAllIsr(rootIsr, wordIsrs, docIsr);
   Location matchLocation = rootIsr->GetCurrentLocation();
   while(matchLocation != IsrGlobals::IsrSentinel)
      {
      //get document info from current location
      Document document(matchLocation, docIsr);
      //move wordIsrs to beginning of doc
      Location docStartLocation = getDocStart(
            document.GetDocInfo().DocumentLength, docIsr);
      seekDecoratedWordIsrs(docStartLocation, wordIsrs);
      //score and rank
      document.ComputeScore(wordIsrs);
      updateTopRankedDocuments(document);
      matchLocation = getNextDocumentLocation(rootIsr, document.GetDocEndLocation());
      }
   return topRankedDocuments;
   }

Location Ranker::Document::GetDocEndLocation()
   {
   return docEndLocation;
   }

Ranker::Document::Document(Location matchLocation, IsrEndDoc* docIsr)
   {
   docEndLocation = docIsr->SeekToLocation(matchLocation);
   if(docEndLocation == IsrGlobals::IsrSentinel)
      {
      fprintf(stderr, "isrWord has no doc end!");
      throw(1);
      }
   docInfo = docIsr->GetDocInfo();
   //TODO: uncomment when Index is complete
   //docInfo = IndexInterface::GetDocumentAttributes(endDocLocation);
   decorationFeatures.AnchorTextFeatures.SetFeatureType(anchor);
   decorationFeatures.BodyTextFeatures.SetFeatureType(body);
   decorationFeatures.TitleFeatures.SetFeatureType(title);
   decorationFeatures.UrlFeatures.SetFeatureType(url);

   decorationFeatures.AnchorTextFeatures.SetCurrentDocument(this);
   decorationFeatures.BodyTextFeatures.SetCurrentDocument(this);
   decorationFeatures.TitleFeatures.SetCurrentDocument(this);
   decorationFeatures.UrlFeatures.SetCurrentDocument(this);
   }

unsigned Ranker::Document::ComputeScore(DecoratedWordIsrs& wordIsrs)
   {
   unsigned anchorScore = decorationFeatures.AnchorTextFeatures.ComputeScore(
         wordIsrs.AnchorTextIsrs);
   unsigned bodyScore = decorationFeatures.BodyTextFeatures.ComputeScore(
         wordIsrs.BodyTextIsrs);
   unsigned titleScore = decorationFeatures.TitleFeatures.ComputeScore(
         wordIsrs.TitleIsrs);
   unsigned urlScore = decorationFeatures.UrlFeatures.ComputeScore(
         wordIsrs.UrlIsrs);

   //computeScore already applies weights for lienar combination
   score = anchorScore + bodyScore + titleScore + urlScore;
   return score;
   }

DocumentAttributes Ranker::Document::GetDocInfo()
   {
   return docInfo;
   }

Ranker::Document::Features::WordStatistics::WordStatistics(Isr* isrIn,
      Document* curDocumentIn)
   : isr(isrIn), curDocument(curDocumentIn), Count(0) {}

unsigned Ranker::Document::Features::ComputeScore(Vector<Isr*>& wordIsrs)
   {
   computeFeatures(wordIsrs);
   unsigned score = textTypeWeight * RankerParams::WordFrequencyWeight * 
         TotalWordFrequency;
   return score;
   }

unsigned Isr::GetImportance()
   {
   return importance; 
   }

WordStatistcs* Ranker::Document::Features::getMostImportantWord(
      Vector<WordStatistics>& wordStatistics)
   {
   unsigned maxImportance = 0;
   WordStatistics* mostImportantWord = nullptr;
   for(int i = 0; i < wordStatistics.size(); ++i)
      {
      Isr* curIsr = wordStatistcs[i].isr;
      if(curIsr->GetImportance() > maxImportance)
         {
         maxImportance = curIsr->GetImportance();
         mostImportanceWord = &WordStatistics[i]; 
         } 
      }
   return mostImportantWord;
   }

void Ranker::getLocationDist(Location location1, Location location2)
   {
   if(location2 > location1)
      return location2 - location1;
   return location1 - location2;
   }

Location Ranker::Document::Features::moveToClosestPosition(WordStatistics* word, 
      WordStatistics* anchorStats)
   {
   Isr* anchor = anchorStats->isr; 
   if(word == anchorStats)
      return anchor->GetCurrentLocation();
   if(word->IsPastEnd())
      return IsrGlobals::IsrSentinel;
      
   Location minDist = getLocationDist(word->isr->curLocation, anchor->curLocation);
   Location closestLocation = word->isr->curLocation;
   while(!word->isPastEnd() && word->isr->curLocation < anchor->curLocation)
      {
      Location dist = getLocationDist(word->isr->curLocation, anchor->curLocation);
      if(dist < minDist)
         {
         minDist = dist;
         closestLocation = word->isr->curLocation; 
         }
      word->SeekNextInstance();
      }
   return closestLocation;
   }

void Ranker::Document::Features::getClosestLocationOrdering(Vector<WordStatistics>& 
      WordStatistics, WordStatistics* anchor, Vector<Location>& closestLocationOrdering)
   {
   for(int i = 0; i < wordIsrs.size(); ++i)
      closestLocationOrder.push_back(getClosestPosition(wordStatistics[i], anchor));
   }

void Ranker::Document::Features::computeSpanFeatures(Vector<Location>& 
   closestLocationOrdering, Vector<WordStatistics*>& wordStatisitcs)
      {
      size_t numWordsInSpan = wordStatistics.size();

      if(numWordsInSpan == 0)
         return;

      Location prevLocation = closestLocationOrdering[0];
      Location leftMostLocation = closestLocationOrdering[0];
      Location rightMostLocation = closestLocationOrdering[0];
      for(int i = 1; i < numWordsInSpan; ++i)
         {
         Location curLocation = closestLocationOrdering[i];
         if(curLocation != IsrGlobals)
            {
            if(curLocation < leftMostLocation)
               leftMostLocation = curLocation;
            else if(curLocation > rightMostLocation)
               rightMostLocation = curLocation;
            if(curLocation < prevLocation)
               NumQueriesOutOfOrder++;
            prevLocation = curLocation;
            }         
         }
      SpanLength = rightMostLocation - leftMostLocation;
      }

void Ranker::Document::Features::computeFeatures(Vector<Isr*> wordIsrs)
   {
   if(wordIsrs.empty())
      return;
   //initialize Vector<WordStatistics>
   //one for each word
   Vector<WordStatistics> wordsStatistics;
   for(size_t i = 0; i < wordIsrs.size(); ++i)
      {
      WordStatistics currentWordStatistic(wordIsrs[i], curDocument);
      wordsStatistics.push_back(currentWordStatistic);
      }

   //note: keep in mind that wordISRs are currently on one of their words
   bool allIsrsPastEnd = false;
   WordStatistics* anchorWord = getMostImportantWord(wordStatistics);
   //corresponds to wordIsrs vec
   Vector<Location> closestLocationOrdering;
   while(!isPastEnd(anchorWord))
      {
      getClosestLocationOrdering(wordStatistics, anchorWord, closestLocationOrdering);
      computeSpanFeatures(closestLocationOrdering, wordsStatistcs);
      anchorWord->SeekNextInstance();
      }
   //seek the rest past end of doc
   while(!allIsrsPastEnd)
      {
      //calculate term counts. TODO: expand to compute more features in this pass
      for(size_t i = 0; i < wordsStatistics.size(); ++i)
         {
         allIsrsPastEnd = true;
         if(!wordsStatistics[i].IsPastEnd())
            {
            wordsStatistics[i].SeekNextInstance();
            allIsrsPastEnd = false;
            }
         }
      }
   
   //sum word statistics
   TotalWordFrequency = 0;
   for(size_t i = 0; i < wordsStatistics.size(); ++i)
      {
      TotalWordFrequency += wordsStatistics[i].Count;
      }
   }

void Ranker::Document::Features::WordStatistics::SeekNextInstance()
   {
   if(!IsPastEnd())
      {
      isr->NextInstance();
      Count++;
      }
   }

bool Ranker::Document::Features::WordStatistics::IsPastEnd()
   {
   return isr->GetCurrentLocation() >= curDocument->docEndLocation
         || isr->GetCurrentLocation() == IsrGlobals::IsrSentinel;
   }

bool Ranker::Document::Features::isPastEnd(Isr* isr)
   {
   return isr->GetCurrentLocation() >= curDocument->docEndLocation
         || isr->GetCurrentLocation() == IsrGlobals::IsrSentinel;
   }

void Ranker::Document::Features::SetFeatureType(TextType textTypeIn)
   {
   textType = textTypeIn;
   if(textType == anchor)
      {
      //anchor specific weights
      textTypeWeight = RankerParams::AnchorWeight;
      }
   else if(textType == body)
      {
      //anchor specific weights
      textTypeWeight = RankerParams::BodyWeight;
      }
   else if(textType == title)
      {
      //anchor specific weights
      textTypeWeight = RankerParams::TitleWeight;
      }
   else //url
      {
      //anchor specific weights
      textTypeWeight = RankerParams::UrlWeight;
      }
   }

unsigned Ranker::Document::GetScore()
   {
   return score;
   }

Ranker::Ranker()
   : numDocsToDisplay(10) {}

bool Ranker::ScoredDocumentCompare::operator() (ScoredDocument doc1, 
      ScoredDocument doc2)
   {
   return doc1.score > doc2.score;
   }

void Ranker::Document::Features::SetCurrentDocument(Document* curDocumentIn)
   {
   curDocument = curDocumentIn; 
   }