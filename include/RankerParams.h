#pragma once
#ifndef RANKERPARAMS_H
#define RANKERPARAMS_H

#include "vector.h"
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

   //feature weights
   extern const unsigned WordFrequencyWeight;
   const unsigned SpanLengthWeight;
   const unsigned SpanOrderednessWeight;
   }

#endif