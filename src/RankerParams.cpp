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

   const unsigned SpanLengthWeight = 1;
   const Vector<CutoffDouble> SpanLengthCutoff = {{1,2}, {2,3}};

   const unsigned QueriesOutOfOrderRatio = 1;
   
   }