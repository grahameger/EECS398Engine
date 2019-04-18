#ifndef ISR_H
#define ISR_H

#include "vector.h"

class String;
class PostingList;

class ISR
   {
   public:
      ISR( String& query );

      virtual unsigned long long NextInstance( unsigned long long after = 0 );

   protected:
      ISR( );

      Vector< ISR* >* children;

   };


class WordISR : ISR
   {
   public:
      WordISR( String& word );

      unsigned long long NextInstance( unsigned long long after = 0 ) override;

   private:
      Vector< PostingList* > postingLists;
      unsigned nextPtr;

   };

#endif
