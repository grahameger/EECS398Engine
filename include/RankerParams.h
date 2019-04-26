#pragma once
#ifndef RANKERPARAMS_H
#define RANKERPARAMS_H

#include "vector.h"
#include <climits>
#include <cfloat>

namespace RankerParams
   {
   struct CutoffFloat
      {
      float upperBound;
      unsigned score;
      };

   struct CutoffInt
      {
      unsigned upperBound;
      unsigned score;
      };

   extern const unsigned AnchorWeight;
   extern const unsigned UrlWeight;
   extern const unsigned BodyWeight;
   extern const unsigned TitleWeight;
   extern const unsigned SpanLengthWeight;
   extern const unsigned SpanOrderednessWeight;

   //feature weights and cutoffs
   extern const unsigned WordFrequencyWeight;
   extern const CutoffInt CutoffWordFreq1;
   extern const CutoffInt CutoffWordFreq2;
   extern const CutoffInt CutoffWordFreq3;
   extern const CutoffInt CutoffWordFreqFinal;
   extern const Vector<CutoffInt> WordFrequencyCutoff;

   //TODO adjust for the fact that same order for many words
   //is much better than same order for fewer!
   extern const CutoffFloat CutoffSpanLen1;
   extern const CutoffFloat CutoffSpanLen2;
   extern const CutoffFloat CutoffSpanLenFinal;
   extern const Vector<CutoffFloat> SpanLengthCutoff;
   extern const unsigned SpanLengthWeight;

   extern const CutoffFloat CutoffSpanOrderedness1;
   extern const CutoffFloat CutoffSpanOrderedness2;
   extern const CutoffFloat CutoffSpanOrderednessFinal;
   extern const Vector<CutoffFloat> SpanOrderednessCutoff;
   extern const unsigned SpanOrderednessWeight;

   extern const CutoffInt CutoffTfidf1;
   extern const CutoffInt CutoffTfidf2;
   extern const CutoffInt CutoffTfidfFinal;
   extern const Vector<CutoffInt> CutoffTfidf;
   extern const unsigned TfidfWeight;

   extern const CutoffInt CutoffDomainPopularity1;
   extern const CutoffInt CutoffDomainPopularity2;
   extern const CutoffInt CutoffDomainPopularityFinal;
   extern const Vector<CutoffInt> CutoffDomainPopularity;
   extern const unsigned DomainPopularityWeight;

   extern const CutoffInt CutoffDomainPopularity1;
   extern const CutoffInt CutoffDomainPopularity2;
   extern const CutoffInt CutoffDomainPopularityFinal;
   extern const Vector<CutoffInt> CutoffDomainPopularity;
   extern const unsigned Okapibm25Weight;

   extern const CutoffInt CutoffNumUrlSlashes1;
   extern const CutoffInt CutoffNumUrlSlashes2;
   extern const CutoffInt CutoffNumUrlSlashesFinal;
   extern const Vector<CutoffInt> CutoffNumUrlSlashes;
   extern const unsigned NumUrlSlashesWeight;

   extern const CutoffInt CutoffNumOutlinks1;
   extern const CutoffInt CutoffNumOutlinks2;
   extern const CutoffInt CutoffNumOutlinksFinal;
   extern const Vector<CutoffInt> CutoffNumOutlinks;
   extern const unsigned NumOutlinksWeight;

   extern const CutoffInt CutoffUrlLength1;
   extern const CutoffInt CutoffUrlLength2;
   extern const CutoffInt CutoffUrlLengthFinal;
   extern const Vector<CutoffInt> CutoffUrlLength;
   extern const unsigned UrlLengthWeight;

   extern const CutoffInt CutoffDomainType1;
   extern const CutoffInt CutoffDomainType2;
   extern const CutoffInt CutoffDomainTypeFinal;
   extern const Vector<CutoffInt> CutoffDomainType;
   extern const unsigned DomainTypeWeight;

   extern const CutoffFloat CutoffSpan1;
   extern const CutoffFloat CutoffSpan2;
   extern const CutoffFloat CutoffSpan3;
   extern const CutoffFloat CutoffSpan4;
   extern const CutoffFloat CutoffSpanFinal;
   extern const Vector<CutoffFloat> CutoffSpan;
   extern const unsigned SpanWeight;
   }

#endif