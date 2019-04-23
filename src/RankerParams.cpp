#include "RankerParams.h"
namespace RankerParams
   {
   const unsigned AnchorWeight = 1;
   const unsigned UrlWeight = 1;
   const unsigned BodyWeight = 1;
   const unsigned TitleWeight = 1;
   const unsigned SpanLengthWeight = 1;
   const unsigned SpanOrderednessWeight = 1;

   //feature weights and cutoffs
   const unsigned WordFrequencyWeight = 1;
   CutoffInt cutoffWordFreq1 = {1, 1};
   CutoffInt cutoffWordFreq2 = {3, 1};
   CutoffInt cutoffWordFreqFinal = {UINT_MAX, 1};
   const Vector<CutoffInt> WordFrequencyCutoff = {cutoff1, cutoff2, cutoffFinal};
   //1 for spanlength and queriesoutoforder is exact match
   const unsigned SpanLengthWeight = 1;
   CutoffInt cutoffSpanLen1 = {1, 1};
   CutoffInt cutoffSpanLen2 = {3, 1};
   CutoffInt cutoffSpanLenFinal = {UINT_MAX, 1};
   const Vector<CutoffInt> SpanLengthCutoff = {cutoffSpanLen1, 
        cutoffSpanLen2, cutoffSpanLenfinal}; 

   const unsigned QueriesOutOfOrderRatio = 1;
   const Vector<CutoffInt> SpanOrderednessCutoff = {{1,1}, {2, 1}, {UINT_MAX, 1}};   
   }