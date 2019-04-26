#define BOOST_TEST_MODULE RankerTest
#include <boost/test/included/unit_test.hpp>

#include "Ranker.h"
#include <cassert>
#include <iostream>

using std::cout;
using std::endl;

void Ranker::Document::Features::testThresholdedInt()
   {
   unsigned score = getThresholdedIntScore(RankerParams::WordFrequencyCutoff, 2);
   BOOST_CHECK_EQUAL(score, 3);

   score = getThresholdedIntScore(RankerParams::WordFrequencyCutoff, 4);
   BOOST_CHECK_EQUAL(score, 4);

   score = getThresholdedIntScore(RankerParams::WordFrequencyCutoff, 50);
   BOOST_CHECK_EQUAL(score, 99);
   }

void Ranker::Document::Features::testComputeFeatures()
   {
   //vector<location> dennislocations = {1, 4, 5, 8, 11};
   //Vector<Location> dennisLocations = {2, 5, 11, 15};
   Vector<Location> dennisLocations = {1};
   Isr isr(dennisLocations);
   isr.SetImportance(2);

   //Vector<Location> liLocations = {3, 6, 7, 14};
   //Vector<Location> liLocations = {3, 9, 16, 18, 19};
   //Vector<Location> liLocations = {3, 9, 16, 18, 19};
   Vector<Location> liLocations = {};
   Isr isr2(liLocations);
   isr2.SetImportance(1);
   
   Vector<Isr*> words = {&isr, &isr2};
   //computeFeatures(words);
   cout << ComputeScore(words) << endl;
   //cout << spanScore << endl;
   }

void Ranker::RunTests()
   {
   Vector<Location> docEndLocations = {10, 20, 30, 40, 50};
   IsrEndDoc docEnd(docEndLocations);

   Location match = 1;
   Document document(match, &docEnd);
   document.RunTests();
   }

void Ranker::Document::Features::RunTests()
   {
   testThresholdedInt();
   testComputeFeatures();
   }

void Ranker::Document::RunTests()
   {
   Ranker::Document::Features features;
   features.SetCurrentDocument(this);
   features.SetFeatureType(anchor);
   features.RunTests();
   }

BOOST_AUTO_TEST_CASE(RankerTest)
   {
   Ranker rank;
   rank.RunTests();
   }