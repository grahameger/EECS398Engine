#include <iostream>
#include <cstdlib>
#include <string>

#include "Postings.h"
#include "vector.h"
#include "ISR.h"

using namespace std;

void AddCommand( );
void ISRCommand( );
void ISRNextCommand( IsrWord& wordISR );
void ISRSeekCommand( IsrWord& wordISR );
void ISRCurrentCommand( IsrWord& wordISR );
void UnknownCommand( );

int main( )
   {
   bool running = true;

   while ( running )
      {
      string command;
      cout << "Enter a command: ";
      cin >> command;

      if ( command == "add" )
         AddCommand( );
      else if ( command == "isr" )
         ISRCommand( );
      else if ( command == "quit" )
         running = false;
      else
         UnknownCommand( );
      }
   }

void AddCommand( )
   {
   string word;
   cout << "What word do you want to add to? ";
   cin >> word;

   unsigned count;
   cout << "How many posts do you want to add to this word? ";
   cin >> count;

   unsigned long long start;
   cout << "Where should these postings start? ";
   cin >> start;

   bool randomized;
   cout << "Should these postings be random? ";
   cin >> randomized;

   Vector< unsigned long long > postings;
   if ( randomized )
      {
      unsigned seed = 792;
      bool pseudo;
      cout << "Pseudo-random? ";
      cin >> pseudo;
      if ( !pseudo )
         seed = time( nullptr );
      srand( seed );

      int maxDistance;
      cout << "What should be the maximum distance between postings? ";
      cin >> maxDistance;

      for ( unsigned i = 0; i < count; i++ )
         postings.push_back( start += ( rand( ) % maxDistance + 1 ) );
      }
   else
      {
      unsigned long long delta;
      cout << "What is the delta between each posting? ";
      cin >> delta;

      for ( unsigned i = 0; i < count; i++ )
         postings.push_back( start += delta );
      }

   Postings::GetPostings( )->AddPostings( word.c_str( ), &postings );

   cout << "Postings added!\n" << endl;
   }


void ISRCommand( )
   {
   string word;
   cout << "What word do you want an ISR for? ";
   cin >> word;

   IsrWord wordISR( word.c_str( ) );

   if ( !wordISR )
      {
      cout << "Could not find a posting list for that word...\n" << endl;
      return;
      }

   bool usingISR = true;
   while ( usingISR )
      {
      string command;
      cout << "Enter an ISR command: ";
      cin >> command;

      if ( command == "next" )
         ISRNextCommand( wordISR );
      else if ( command == "seek" )
         ISRSeekCommand( wordISR );
      else if ( command == "current" )
         ISRCurrentCommand( wordISR );
      else if ( command == "quit" )
         usingISR = false;
      else
         UnknownCommand( );
      }
   }


void ISRNextCommand( IsrWord& wordISR )
   {
   cout << "The next occurence of this word is at ";
   cout << wordISR.NextInstance( ) << ".\n" << endl;
   }


void ISRSeekCommand( IsrWord& wordISR )
   {
   unsigned long long after;
   cout << "Get the first posting after what location? ";
   cin >> after;

   cout << "The first posting larger than that is at ";
   cout << wordISR.SeekToLocation( after ) << ".\n" << endl;
   }


void ISRCurrentCommand( IsrWord& wordISR )
   {
   cout << "This ISR is currently on post ";
   cout << wordISR.CurInstance( ) << ".\n" << endl;
   }


void UnknownCommand( )
   {
   cout << "Command not recognized...\n" << endl;
   cin.ignore( numeric_limits< streamsize >::max( ), '\n' );
   }
