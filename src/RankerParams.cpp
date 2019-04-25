#include "RankerParams.h"
namespace RankerParams
   {
   const unsigned AnchorWeight = 1;
   const unsigned UrlWeight = 1;
   const unsigned BodyWeight = 1;
   const unsigned TitleWeight = 1;

   //feature weights and cutoffs
   const unsigned WordFrequencyWeight = 1;
   const CutoffInt CutoffWordFreq1 = {1, 1};
   const CutoffInt CutoffWordFreq2 = {3, 1};
   const CutoffInt CutoffWordFreqFinal = {UINT_MAX, 1};
   const Vector<CutoffInt> WordFrequencyCutoff = {CutoffWordFreq1, CutoffWordFreq2,
            CutoffWordFreqFinal};
   //1 for spanlength and queriesoutoforder is exact match
   const CutoffInt CutoffSpanLen1 = {1, 1};
   const CutoffInt CutoffSpanLen2 = {3, 1};
   const CutoffInt CutoffSpanLenFinal = {UINT_MAX, 1};
   const Vector<CutoffInt> SpanLengthCutoff = {CutoffSpanLen1, 
        CutoffSpanLen2, CutoffSpanLenFinal}; 
   const unsigned SpanLengthWeight = 1;

   const CutoffInt CutoffSpanOrderedness1 = {1, 1};
   const CutoffInt CutoffSpanOrderedness2 = {3, 1};
   const CutoffInt CutoffSpanOrderednessFinal = {UINT_MAX, 1};
   const Vector<CutoffInt> SpanOrderednessCutoff = {CutoffSpanOrderedness1, 
        CutoffSpanOrderedness2, CutoffSpanOrderednessFinal}; 
   const unsigned SpanOrderednessWeight = 1;

   const CutoffInt CutoffTfidf1 = {1, 1};
   const CutoffInt CutoffTfidf2 = {3, 1};
   const CutoffInt CutoffTfidfFinal = {UINT_MAX, 1};
   const Vector<CutoffInt> CutoffTfidf = {CutoffTfidf1, 
        CutoffTfidf2, CutoffTfidfFinal}; 
   const unsigned TfidfWeight = 1;

   const CutoffInt CutoffDomainPopularity1 = {1, 1};
   const CutoffInt CutoffDomainPopularity2 = {3, 1};
   const CutoffInt CutoffDomainPopularityFinal = {UINT_MAX, 1};
   const Vector<CutoffInt> CutoffDomainPopularity =
         {CutoffDomainPopularity1, CutoffDomainPopularity2, CutoffDomainPopularityFinal};
   const unsigned DomainPopularityWeight = 1;


   const CutoffInt CutoffOkapi1 = {1, 1};
   const CutoffInt CutoffOkapi2 = {3, 1};
   const CutoffInt CutoffOkapiFinal = {UINT_MAX, 1};
   const Vector<CutoffInt> CutoffOkapi =
         {CutoffOkapi1, CutoffOkapi2, CutoffOkapiFinal};
   const unsigned Okapibm25Weight = 1;

   const CutoffInt CutoffNumUrlSlashes1 = {1, 1};
   const CutoffInt CutoffNumUrlSlashes2 = {3, 1};
   const CutoffInt CutoffNumUrlSlashesFinal = {UINT_MAX, 1};
   const Vector<CutoffInt> CutoffNumUrlSlashes =
         {CutoffNumUrlSlashes1, CutoffNumUrlSlashes2, CutoffNumUrlSlashesFinal};
   const unsigned NumUrlSlashesWeight = 1;

   const CutoffInt CutoffNumOutlinks1 = {1, 1};
   const CutoffInt CutoffNumOutlinks2 = {3, 1};
   const CutoffInt CutoffNumOutlinksFinal = {UINT_MAX, 1};
   const Vector<CutoffInt> CutoffNumOutlinks = {CutoffNumOutlinks1, 
         CutoffNumOutlinks2, CutoffNumOutlinksFinal};
   const unsigned NumOutlinksWeight = 1;

   const CutoffInt CutoffUrlLength1 = {1, 1};
   const CutoffInt CutoffUrlLength2 = {3, 1};
   const CutoffInt CutoffUrlLengthFinal = {UINT_MAX, 1};
   const Vector<CutoffInt> CutoffUrlLength = {CutoffUrlLength1, 
         CutoffUrlLength2, CutoffUrlLengthFinal};
   const unsigned UrlLengthWeight = 1;

   const CutoffInt CutoffDomainType1 = {1, 1};
   const CutoffInt CutoffDomainType2 = {3, 1};
   const CutoffInt CutoffDomainTypeFinal = {UINT_MAX, 1};
   const Vector<CutoffInt> CutoffDomainType = {CutoffDomainType1, 
         CutoffDomainType2, CutoffDomainTypeFinal};
   const unsigned DomainTypeWeight = 1;

   const CutoffFloat CutoffSpan1 = {1, 1};
   const CutoffFloat CutoffSpan2 = {3, 1};
   const CutoffFloat CutoffSpanFinal = {FLT_MAX, 1};
   const Vector<CutoffFloat> CutoffSpan = {CutoffSpan1,
          CutoffSpan2, CutoffSpanFinal};
   const unsigned SpanWeight = 1;
   }