#ifndef ISR_H
#define ISR_H

#include "Postings.h"
#include "vector.h"

class String;
class PostingList;


using Location = unsigned long long;


class IsrWord
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


class IsrEndDoc : public IsrWord
   {
   public:
      IsrEndDoc( );

   };

#endif
