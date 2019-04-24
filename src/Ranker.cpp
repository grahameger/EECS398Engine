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

Ranker::Document::Features::Features()
   : spanLength(0), numQueriesOutOfOrder(0), totalWordFrequency(0) {}

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

float Ranker::Document::Features::getWordFrequencyScore()
   {
   return RankerParams::WordFrequencyWeight * getThresholdedIntScore(
         RankerParams::WordFrequencyCutoff, totalWordFrequency) / (float) numQueryWords;
   }

float Ranker::Document::Features::getSpanLengthScore()
   {
   return RankerParams::SpanLengthWeight * getThresholdedIntScore(
         RankerParams::CutoffQueriesOutOfOrder, spanLength) / (float) numQueryWords;
   }

float Ranker::Document::Features::getSpanOrderednessScore()
   {
   return RankerParams::SpanOrderednessWeight * getThresholdedIntScore(
         RankerParams::CutoffSpanOrderedness, numQueriesOutOfOrder) / 
         (float) numQueryWords;
   }

unsigned Ranker::Document::Features::ComputeScore(Vector<Isr*>& wordIsrs)
   {
   numQueryWords = wordIsrs.size();
   computeFeatures(wordIsrs);
   float wordFrequencyScore = getWordFrequencyScore();
   float spanLengthScore = getSpanLengthScore();
   float spanOrderednessScore = getSpanOrderednessScore();
   unsigned score = (unsigned) textTypeWeight * (wordFrequencyScore + spanLengthScore
         + spanOrderednessScore);
   return score;
   }

unsigned Isr::GetImportance()
   {
   return importance; 
   }

unsigned Ranker::Document::Features::getThresholdedIntScore(
      Vector<RankerParams::CutoffInt>& cutoffs, unsigned featureValue)
   {
   if(cutoffs.empty())
      {
      printf("cutoffs is empty!");
      exit(1);
      }

   unsigned score = cutoffs[0].score;
   for(size_t i = 0; i < cutoffs.size(); ++i)
      {
      RankerParams::CutoffInt& cutoff = cutoffs[i];
      if(featureValue <= cutoff.upperBound)
         score = cutoff.score;
      else
         break;
      }
   
   return score;
   }

unsigned Ranker::Document::Features::getThresholdedFloatScore(
      Vector<RankerParams::CutoffFloat>& cutoffs, float featureValue)
   {
   if(cutoffs.empty())
      {
      printf("cutoffs is empty!");
      exit(1);
      }

   unsigned score = cutoffs[0].score;
   for(size_t i = 0; i < cutoffs.size(); ++i)
      {
      RankerParams::CutoffFloat& cutoff = cutoffs[i];
      if(featureValue <= cutoff.upperBound)
         score = cutoff.score;
      else
         break;
      }
   
   return score;
   }


WordStatistics* Ranker::Document::Features::getMostImportantWord(
      Vector<WordStatistics>& wordStatistics)
   {
   unsigned maxImportance = 0;
   WordStatistics* mostImportantWord = nullptr;
   for(int i = 0; i < wordStatistics.size(); ++i)
      {
      Isr* curIsr = wordStatistics[i].isr;
      if(curIsr->GetImportance() > maxImportance)
         {
         maxImportance = curIsr->GetImportance();
         mostImportantWord = &wordStatistics[i]; 
         } 
      }
   return mostImportantWord;
   }

Location Ranker::getLocationDist(Location location1, Location location2)
   {
   if(location2 > location1)
      return location2 - location1;
   return location1 - location2;
   }

Location Ranker::Document::Features::getClosestPosition(WordStatistics* word, 
      WordStatistics* anchorStats)
   {
   Isr* anchor = anchorStats->isr; 
   if(word == anchorStats)
      return anchor->GetCurrentLocation();
   if(word->IsPastEnd())
      return IsrGlobals::IsrSentinel;
      
   Location minDist = getLocationDist(word->isr->GetCurrentLocation(), anchor->GetCurrentLocation());
   Location closestLocation = word->isr->GetCurrentLocation();
   while(!word->IsPastEnd() && word->isr->GetCurrentLocation() < anchor->GetCurrentLocation())
      {
      Location dist = getLocationDist(word->isr->GetCurrentLocation(), anchor->GetCurrentLocation());
      if(dist < minDist)
         {
         minDist = dist;
         closestLocation = word->isr->GetCurrentLocation(); 
         }
      word->SeekNextInstance();
      }
   return closestLocation;
   }

void Ranker::Document::Features::getClosestLocationOrdering(Vector<WordStatistics>& 
      wordStatistics, WordStatistics* anchor, Vector<Location>& closestLocationOrdering)
   {
   for(int i = 0; i < wordStatistics.size(); ++i)
      closestLocationOrdering.push_back(getClosestPosition(wordStatistics[i], anchor));
   }

void Ranker::Document::Features::computeSpanFeatures(Vector<Location>& 
      closestLocationOrdering, Vector<WordStatistics>& wordStatistics)
   {
   size_t numWordsInSpan = wordStatistics.size();
   numQueriesOutOfOrder = 0;

   if(numWordsInSpan == 0)
      return;

   Location prevLocation = closestLocationOrdering[0];
   Location leftMostLocation = closestLocationOrdering[0];
   Location rightMostLocation = closestLocationOrdering[0];
   for(int i = 1; i < numWordsInSpan; ++i)
      {
      Location curLocation = closestLocationOrdering[i];
      if(curLocation != IsrGlobals::IsrSentinel)
         {
         if(curLocation < leftMostLocation)
            leftMostLocation = curLocation;
         else if(curLocation > rightMostLocation)
            rightMostLocation = curLocation;
         if(curLocation < prevLocation)
            numQueriesOutOfOrder++;
         prevLocation = curLocation;
         }         
      }

   spanLength = rightMostLocation - leftMostLocation + 1;
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
   WordStatistics* anchorWord = getMostImportantWord(wordsStatistics);
   //corresponds to wordIsrs vec
   Vector<Location> closestLocationOrdering;
   while(!isPastEnd(anchorWord->isr))
      {
      getClosestLocationOrdering(wordsStatistics, anchorWord, closestLocationOrdering);
      computeSpanFeatures(closestLocationOrdering, wordsStatistics);
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
   totalWordFrequency = 0;
   for(size_t i = 0; i < wordsStatistics.size(); ++i)
      {
      totalWordFrequency += wordsStatistics[i].Count;
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