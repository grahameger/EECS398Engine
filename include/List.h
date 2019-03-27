#pragma once
#ifndef LIST_H
#define LIST_H

template < typename T >
class Node;

template < typename T >
struct PointerLess { typedef T type; };

template < typename T >
struct PointerLess< T* > { typedef T type; };

template < typename T >
class List
   {
public:
   typedef typename PointerLess< T >::type baseType;

   class Iterator
      {
   public:
      Iterator( );
      void operator=( const Iterator& toCopy );

      const baseType& operator[ ] ( int index );

      bool operator!= ( Iterator rhs );
      bool operator== ( Iterator rhs );

      Iterator& operator++ ( );
      Iterator operator++ ( int postfix );
      Iterator& operator-- ( );
      Iterator operator-- ( int postfix );

   private:
      Iterator( Node< T >* node );
      Node< T >* node;

      friend Iterator List< T >::GetFront( );
      friend Iterator List< T >::GetBack( );

      };

   List( );
   ~List( );
   List( const List& toCopy ) = delete;
   List( List&& toMove ) = delete;
   List< T >& operator=( const List& toCopy ) = delete;
   List< T >& operator=( List&& toMove ) = delete;

   bool Empty( );

   void AddToFront( T toAdd );
   void AddToBack( T toAdd );

   Iterator GetFront( );
   Iterator GetBack( );

   T&& RemoveFront( );
   T&& RemoveBack( );


private:
   Node< T > *front, *back;

   };

#endif
