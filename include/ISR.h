#ifndef ISR_H
#define ISR_H

#include "vector.h"

class String;
class PostingList;


using Location = unsigned long long;


class IsrWord
   {
   public:
      IsrWord( String word );

      Location NextInstance( );
      Location SeekToLocation( Location seekDestination = 0 );
      Location CurInstance( ) const;

      operator bool( ) const;

   private:
      bool validISR;
      Location currentLocation;

      unsigned nextPtr;
      Vector< PostingList* > postingLists;

   };


class IsrEndDoc : IsrWord
   {
   public:
      IsrEndDoc( );

   };

#endif
