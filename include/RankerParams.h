#pragma once
#ifndef RANKERPARAMS_H
#define RANKERPARAMS_H

#include "Vector.h"
namespace RankerParams
   {
   struct CutoffDouble
      {
      double upperBound;
      unsigned score;
      };

   struct CutoffInt
      {
      unsignedk upperBound;
      unsigned score;
      };

   extern const unsigned AnchorWeight;
   extern const unsigned UrlWeight;
   extern const unsigned BodyWeight;
   extern const unsigned TitleWeight;

   //feature weights
   extern const unsigned WordFrequencyWeight;
   const unsigned SpanLengthWeight;
   const unsigned QueriesOutOfOrderRatio;
   }

#endif