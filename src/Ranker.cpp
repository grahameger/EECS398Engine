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
   : totalWordFrequency(0), spanScore(0), numQueryWords(0) {}

Location Ranker::Document::GetDocEndLocation()
   {
   return docEndLocation;
   }

//moves docIsr to location of match
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
   : isr(isrIn), enclosingDocument(curDocumentIn), Count(0) {}

float Ranker::Document::Features::getWordFrequencyScore()
   {
   //TODO REMOVE
   return 0;
   return RankerParams::WordFrequencyWeight * getThresholdedIntScore(
         RankerParams::WordFrequencyCutoff, totalWordFrequency) / (float) numQueryWords;
   }

unsigned Ranker::Document::Features::getSpanScore()
   {
   return RankerParams::SpanWeight * getThresholdedFloatScore(
         RankerParams::CutoffSpan, spanScore);
   }

unsigned Ranker::Document::Features::ComputeScore(Vector<Isr*>& wordIsrs)
   {
   numQueryWords = wordIsrs.size();
   computeFeatures(wordIsrs);
   float wordFrequencyScore = getWordFrequencyScore();
   unsigned spanScore = getSpanScore();

   unsigned score = (unsigned) textTypeWeight * (wordFrequencyScore + spanScore);
   return score;
   }

unsigned Isr::GetImportance()
   {
   return importance; 
   }

unsigned Ranker::Document::Features::getThresholdedIntScore(
      const Vector<RankerParams::CutoffInt>& cutoffs, unsigned featureValue)
   {
   if(cutoffs.empty())
      {
      printf("cutoffs is empty!");
      exit(1);
      }

   size_t cutoffInd = 0;
   RankerParams::CutoffInt& cutoff = cutoffs[cutoffInd];
   while(featureValue > cutoff.upperBound)
      {
      cutoffInd++;
      cutoff = cutoffs[cutoffInd];
      }
   
   return cutoff.score;
   }

unsigned Ranker::Document::Features::getThresholdedFloatScore(
      const Vector<RankerParams::CutoffFloat>& cutoffs, float featureValue)
   {
   if(cutoffs.empty())
      {
      printf("cutoffs is empty!");
      exit(1);
      }

   size_t cutoffInd = 0;
   RankerParams::CutoffFloat& cutoff = cutoffs[cutoffInd];
   while(featureValue > cutoff.upperBound)
      {
      cutoffInd++;
      cutoff = cutoffs[cutoffInd];
      }
   
   return cutoff.score;
   }

Ranker::Document::Features::WordStatistics* 
         Ranker::Document::Features::getMostImportantWord(Vector<WordStatistics>& 
         wordStatistics)
   {
   if(wordStatistics.empty())
      return nullptr;

   unsigned maxImportance = 0;
   WordStatistics* mostImportantWord = &wordStatistics[0];
   for(size_t i = 0; i < wordStatistics.size(); ++i)
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

Location Ranker::Document::Features::getLocationDist(Location location1, Location location2)
   {
   if(location2 > location1)
      return location2 - location1;
   return location1 - location2;
   }

Location Ranker::getLocationDist(Location location1, Location location2)
   {
   if(location2 > location1)
      return location2 - location1;
   return location1 - location2;
   }

Location Ranker::min(Location location1, Location location2)
   {
   if(location1 < location2)
      return location1;
   else
      return location2;
   }

Location Ranker::Document::Features::getClosestPosition(WordStatistics* word, 
      WordStatistics* anchorStats, Location prevClosestLocation)
   {
   Isr* anchor = anchorStats->isr; 
   if(word == anchorStats)
      return anchor->GetCurrentLocation();
   if(word->IsPastEnd())
      return IsrGlobals::IsrSentinel;

   Location curLocationDist = getLocationDist(word->isr->GetCurrentLocation(), anchor->GetCurrentLocation());
   Location prevLocationDist = getLocationDist(prevClosestLocation, anchor->GetCurrentLocation());

   Location minDist = 0;
   Location closestLocation = 0;
   if(curLocationDist < prevLocationDist)
      {
      minDist = curLocationDist;
      closestLocation = word->isr->GetCurrentLocation();
      }
   else
      {
      minDist = prevLocationDist;
      closestLocation = prevClosestLocation;
      }
      

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
   for(size_t i = 0; i < wordStatistics.size(); ++i)
      {
      Location prevClosestLocation = closestLocationOrdering[i];
      closestLocationOrdering[i] = getClosestPosition(&wordStatistics[i], anchor, 
            prevClosestLocation);
      }
   }

void Ranker::Document::Features::updateSpanFeatures(Vector<Location>& 
      closestLocationOrdering, Vector<WordStatistics>& wordStatistics)
   {
   size_t numWordsInSpan = wordStatistics.size();
   unsigned numQueriesOutOfOrder = 0;

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

   if(rightMostLocation < leftMostLocation)
      {
      printf("Rightmost location < leftmost location!");
      exit(1);
      }

   unsigned spanLength = rightMostLocation - leftMostLocation + 1;

   printf("Num out of order: %u\n", numQueriesOutOfOrder);
   printf("SpanLen: %u\n", spanLength);

   unsigned spanLengthScore = getThresholdedFloatScore(RankerParams::SpanLengthCutoff, (spanLength / (float) numQueryWords));
   unsigned orderednessScore = getThresholdedFloatScore(RankerParams::SpanOrderednessCutoff, (numQueriesOutOfOrder / (float) numQueryWords));
   
   //orderedness and length compound each other
   spanScore += spanLengthScore * orderednessScore;   
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
   //Vector<Location> closestLocationOrdering;
   /*
   Vector<Location> anchorLocations;
   while(!isPastEnd(anchorWord->isr))
      {
      anchorLocations.push_back(anchorWord->isr->GetCurrentLocation());
      anchorWord->SeekNextInstance();
      }

   Vector<Location> closestLocationOrdering;
   for(size_t i = 0; i < )
   */
   //one-to-one with wordIsrs vec
   Vector<Location> closestLocationOrdering;
   //initialize to current position of isrs
   for(size_t i = 0; i < wordIsrs.size(); ++i)
      closestLocationOrdering.push_back(wordIsrs[i]->GetCurrentLocation());

   while(!isPastEnd(anchorWord->isr))
      {
      getClosestLocationOrdering(wordsStatistics, anchorWord, closestLocationOrdering);
      PrintVec(closestLocationOrdering);
      updateSpanFeatures(closestLocationOrdering, wordsStatistics);
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
   return isr->GetCurrentLocation() >= enclosingDocument->docEndLocation
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

void PrintVec(Vector<Location>& vec)
   {
   printf("printing vector!");
   for(size_t i = 0; i < vec.size(); ++i)
      printf("%u\n", vec[i]);
   }