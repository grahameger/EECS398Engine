#ifndef ISR_H
#define ISR_H

#include "Postings.h"
#include "vector.h"

class String;
class PostingList;

typedef size_t FileOffset;

namespace IsrGlobals
{
   const Location IsrSentinel = 0;
   //TODO: CHange to 1
   const Location IndexStart = 1;
}

struct DocumentLocation {
    Location docStart;
    Location docEnd;
};


using Location = unsigned long long;

class Isr{
public:
   Isr();

   ~Isr();
   //Isr pointer to the end of the current doc
   Isr* docEnd;
   
   //Find next instance of a term
   virtual Location NextInstance() = 0;
   
   //Similar to nextDocument, finds the first occurrence of a term just at 'target' or later
   virtual Location SeekToLocation(Location target);
   
   //First number in the posting list of a term
   virtual Location GetCurrentLocation();

   virtual Location ResetToStart() = 0;

   virtual Location MoveAllTermsOffCurrentDocument();

   virtual void addTerm(Isr*);
   
   //Returns whatever document you're looking at
   Isr* getDocIsr();
   
   protected:
   Vector<Isr*> terms;
   Location currentLocation;
};



class IsrWord : public Isr
   {
   public:
      IsrWord( String word );
      ~IsrWord( );

      Location NextInstance( );
      Location SeekToLocation( Location seekDestination = 0 );
      Location CurInstance( ) const;

      operator bool( ) const;

   private:
      bool validISR;
      Location currentLocation;

      unsigned nextPtr;
      Vector< PostingList* > postingLists;
      Vector< SubBlock > subBlocks;

   };

class IsrOr : public Isr {
public:
   //Variable to keep track of how many terms are in 'terms' (because resize/reserve isn't implemented)
//   unsigned numOfTerms = 0;
   
   //Constructor for IsrOr, MUST be in a vector<Isr> format, otherwise it wont compile
   IsrOr(Vector<Isr*>& phrasesToInsert);
   IsrOr() {}
   virtual Location ResetToStart() override;
   
   //Find the next instance of ANY of the words in 'terms'
   Location NextInstance() override;
};

class IsrAnd : public Isr {
public:
   //Constructor for IsrAnd
   IsrAnd(Vector<Isr*>& phrasesToInsert);
   IsrAnd() {}
   virtual Location ResetToStart() override;

   //Destructor
   ~IsrAnd(){};
   
   Location NextInstance() override;
   
private:
   Location moveTermsToSameDocument(DocumentLocation& docLocation);
};

class IsrPhrase : public Isr {
public:
    IsrPhrase(Vector<Isr*>& phrasesToInsert);
    IsrPhrase() {}
    virtual Location ResetToStart() override;

    ~IsrPhrase(){};
    Location NextInstance() override;
};

class IsrEndDoc : public IsrWord
   {
   public:
      IsrEndDoc( );

   };

#endif
