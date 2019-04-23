#include "RankerParams.h"
namespace RankerParams
   {
   /*
   const unsigned AnchorWeight = 10;
   const unsigned UrlWeight = 6;
   const unsigned BodyWeight = 1;
   const unsigned TitleWeight = 3;

   //feature weights
   const unsigned WordFrequencyWeight = 1;
   */
   const unsigned AnchorWeight = 1;
   const unsigned UrlWeight = 1;
   const unsigned BodyWeight = 1;
   const unsigned TitleWeight = 1;

   //feature weights and cutoffs
   const unsigned WordFrequencyWeight = 1;
   const Vector<CutoffInt> WordFrequencyCutoff = {{1, 1}, {3, 1}, {UINT_MAX, 1}};
   //1 for spanlength and queriesoutoforder is exact match
   const unsigned SpanLengthWeight = 1;
   const Vector<CutoffFloat> SpanLengthCutoff = {{1,1}, {1.3,1}, {(float) FLT_MAX, 1}}; 

   const unsigned QueriesOutOfOrderRatio = 1;
   const Vector<CutoffFloat> QueriesOutOfOrderCutoff = {{1,1}, {1.3, 1}, {(float) FLT_MAX, 1}};   
   }