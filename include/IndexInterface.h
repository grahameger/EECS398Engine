#pragma once
#ifndef INDEXINTERFACE_H
#define INDEXINTERFACE_H
#include <limits.h>
#include "PersistentHashMap.h"
#include "Postings.h"
#include "index.h"
/*
*feel free to change the datatypes as necessary as long as 
*functionality is the same, e.g. unsigned int-->unsigned short
*/
//Absolute position in index
//Given the Location of an EndDoc, ranker needs access to a map from this Location to
//the corresponding DocumentAttribute
typedef unsigned long long Location;

struct DocumentAttributes
  {
  //Either this or whatever is best to use to identify a page so it can be
  //displayed on front end if it is a top result
  unsigned long long DocID;
  //Length in words. Taking the endDoc Location - DocumentLength
  //should give me the Location of the first word in the document
  unsigned int DocumentLength;
  //Ranking in top sites. 0 == most popular
  unsigned int DomainImportance;
  //If you don't already have this functionality then don't worry about it
  unsigned short NumOutlinks;
  unsigned char NumSlashesInUrl;
  unsigned char NumCharactersInUrl;
  //'c' for com, 'o' for org, 'n' for net, 'i' for int, 'e' for edu, 'g' for gov,
  //'m' for mil, 'u' for any two character domain like .us (catch-all for country codes)
  //'x' for anything else
  char TopLevelDomainType;
  DocumentAttributes(urlMetadata& u):DocID(0), DocumentLength(u.length), DomainImportance(u.importance),
      NumOutlinks(u.outLinks), NumSlashesInUrl(u.urlSlashes), NumCharactersInUrl(u.urlLength)
    {}
  };

struct CorpusAttributes
  {
  //TODO: change type to what it should be
  //
  //Map from word to total number of occurances in corpus.
  //Is this feasible? We should discuss.
  //an alternative could be for me to download a list of the 1mil 
  //most common words on the internet and use that to scale things for rarity 
  //(e.g. in tf-idf) instead of the corpus-based frequency dictionary. 
  //The equation for tf-idf however does specify for 
  //"total instances of query in corpus."
  bool WordFrequency;
  //in words
  unsigned int AverageDocumentLength; 
  };
class IndexInterface{
  DocumentAttributes GetDocumentAttributes(Location endDocLocation);
  IndexInterface();
  unsigned AverageDocumentLength;
private:
  PersistentHashMap< unsigned long long, FixedLengthURL > urlMap;
  PersistentHashMap< FixedLengthURL, urlMetadata > metaMap;

};
#endif