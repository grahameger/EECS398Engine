// Graham Eger added a resize() function that takes an argument on 04/08/2019
// Dennis Li added all variations of insert function on 4/18/19

#pragma once
#ifndef VECTOR_H
#define VECTOR_H

#include <algorithm>
template<class T>
class Vector {
    //OVERVIEW: A container that provides random access to
    //          any element by its index.  Indices start at 0.
    //          The capacity (maximum size) of an IntVector is
    //          100 elements.
    
private:
    
    T *elements; //pointer to dynamic array
    size_t numElements = 0;  //Current capacity
    size_t numAllocated = 400; //capacity of array
    
public:
    
    Vector();
    
    //EFFECTS: Constructor
    Vector(size_t num);
    
    //Custom Destructor;
    ~Vector();
    
    //Copy constructor
    Vector(const Vector<T> &other);
    
    //Assignment operator
    Vector<T> &operator=(const Vector<T> &rhs);
    
    void reserve(size_t n);
    void resize();
    
    void resize(size_t n);
    
    //REQUIRES: this IntVector is not full
    //MODIFIES: this IntVector
    //EFFECTS:  Adds value to the end of this IntVector.
    void push_back(T value);
    
    //REQUIRES: this IntVector is not empty
    //MODIFIES: this IntVector
    //EFFECTS:  Removes the last element in this IntVector.
    void pop_back();

    //MODIFIES: this Vector
    //EFFECTS:  Inserts value into position before index ind
    void insert(T value, size_t ind);

    //MODIFIES: this Vector
    //EFFECTS:  Inserts value into position before index ind. 
    //size stays fixed so last element gets popped after insertion
    void insert_fixed_size(T value, size_t ind);

    //REQUIRES: Vector must be sorted!
    //EFFECTS: Insert element into vector in sorted order
    //Maintains original size, so largest element will be popped!
    template <typename Compare>
    void insertSorted(T value, Compare comparator);

    //REQUIRES: Vector must be sorted!
    //EFFECTS: Insert element into vector in sorted order
    //Will maintain max size of maxSize, so if current size
    //is maxSize the largest element will be popped!
    template <typename Compare>
    void insertSortedMaxSize(T value, Compare comparator, size_t maxSize);

    //REQUIRES: 0 <= index < number of elements in this IntVector
    //EFFECTS:  Returns (by reference) the element at the given index.
    T &at(size_t index);

    const T& back();
    
    //REQUIRES: 0 <= index < number of elements in this IntVector
    //EFFECTS:  Returns (by reference) the element at the given index.
    const T &at(size_t index) const;
    
    //REQUIRES: 0 <= index < number of elements in this IntVector
    //EFFECTS:  Returns (by reference) the element at the given index.
    T& operator[](size_t index);
    
    //EFFECTS:  Returns the number of elements of this IntVector.
    size_t size() const;
    
    size_t capacity() const;
    
    //EFFECTS:  Returns true if this IntVector is empty, false otherwise.
    bool empty() const;
    
    //EFFECTS:  Returns true if this Vector is at capacity, false otherwise.
    //          That is, you may add elements if and only if full() is false.
    bool full() const;
};

//custom constructor
template<class T>
Vector<T>::Vector() {
    elements = new T[numAllocated];
}

template<class T>
Vector<T>::Vector(size_t capacity) {
    numAllocated = capacity;
    elements = new T[numAllocated];
}

//copy constructor
template<class T>
Vector<T>::Vector(const Vector<T> &other) {
    //1. initialize member variables
    elements = new T[other.numAllocated];
    numElements = other.numElements;
    numAllocated = other.numAllocated;
    
    //2. Copy everything over
    for(size_t i = 0; i < other.numElements; i++) {
        elements[i] = other.elements[i];
    }
    
    return;
}

//Assignment operator
template<class T>
Vector<T>& Vector<T>::operator=(const Vector<T> &rhs) {
    if(this == &rhs) {
        return *this; //fix "s = s"
    }
    delete[] elements; //remove all
    
    elements = new T[rhs.numAllocated];
    numElements = rhs.numElements;
    numAllocated = rhs.numAllocated;
    for(size_t i = 0; i < rhs.numElements; i++) {
        elements[i] = rhs.elements[i];
    }
    return *this;
}

//Destructor
template<class T>
Vector<T>::~Vector() {
    delete[] elements;
}

//MODIFIES: this Vector
//EFFECTS:  Adds value to the end of this IntVector.
template<class T>
void Vector<T>::push_back(T value){
    if(full()) {
        resize();
    }
    elements[numElements] = value;
    numElements++;
}

//REQUIRES: 0 <= index < number of elements in this IntVector
//EFFECTS:  Returns (by reference) the element at the given index.
template<class T>
const T &Vector<T>::at(size_t index) const {
    return elements[index];
}

//REQUIRES: 0 <= index < number of elements in this IntVector
//EFFECTS:  Returns (by reference) the element at the given index.
template<class T>
T& Vector<T>::operator[] (size_t index) {
    return elements[index];
}

template<class T>
const T& Vector<T>::back() {
    return elements[this->size() - 1];
}

//EFFECTS:  Returns the number of elements of this IntVector.
template<class T>
size_t Vector<T>::size() const {
    return numElements;
}

//EFFECTS:  Returns the number of elements of this IntVector.
template<class T>
size_t Vector<T>::capacity() const {
    return numAllocated;
}


//EFFECTS:  Returns true if this IntVector is empty, false otherwise.
template<class T>
bool Vector<T>::empty() const {
    return numElements == 0;
}

//EFFECTS:  Returns true if this IntVector is at capacity, false otherwise.
//          That is, you may add elements if and only if full() is false.
template<class T>
bool Vector<T>::full() const {
    return (numElements >= numAllocated);
}

//Resize
template<class T>
void Vector<T>::resize() {
    T * tmp_array = new T[this->numAllocated * 2];
    for (size_t i = 0; i < numElements; i++) {
        tmp_array[i] = elements[i];
    }
    std::swap(tmp_array, elements);
    numAllocated *= 2;
    delete [] tmp_array;
}

//MODIFIES: this Vector
//EFFECTS:  Inserts value into position before index ind
template<class T>
void Vector<T>::insert(T value, size_t ind) {
    if(full()) {
        resize();
    }

    for(size_t i = ind; i < numElements; ++i) {
        T displacedElement = elements[i];
        elements[i] = value;
        value = displacedElement;
    }

    elements[numElements] = value;
    numElements++;
}

//MODIFIES: this Vector
//EFFECTS:  Inserts value into position before index ind. 
//size stays fixed so last element gets popped after insertion
template<class T>
void Vector<T>::insert_fixed_size(T value, size_t ind) {
    for(size_t i = ind; i < numElements; ++i) {
        T displacedElement = elements[i];
        elements[i] = value;
        value = displacedElement;
    }
}


//REQUIRES: Vector must be sorted!
//EFFECTS: Insert element into vector in sorted order
//Maintains original size, so largest element will be popped!
template<class T>
template<class Compare>
void Vector<T>::insertSorted(T value, Compare comparator) {
    for(size_t i = 0; i < numElements; ++i) {
        if(comparator(value, elements[i])) {
            insert_fixed_size(value, i);
            break;
        }
    }
}

template<class T>
void Vector<T>::pop_back() {
   numElements--;
}

//REQUIRES: Vector must be sorted!
//EFFECTS: Insert element into vector in sorted order
//Will maintain max size of maxSize, so if current size
//is maxSize the largest element will be popped!
template<class T>
template<class Compare>
void Vector<T>::insertSortedMaxSize(T value, Compare comparator, size_t maxSize) {
    bool insertedElement = false;
    for(size_t i = 0; i < numElements && !insertedElement; ++i) {
        if(comparator(value, elements[i])) {
            insert(value, i);
            insertedElement = true;
        }
    }

    if(!insertedElement)
       push_back(value);

    if(numElements > maxSize)
       pop_back();
}

#endif