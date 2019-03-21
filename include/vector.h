template<class T>
class Vector {
    //OVERVIEW: A container that provides random access to
    //          any element by its index.  Indices start at 0.
    //          The capacity (maximum size) of an IntVector is
    //          100 elements.
    
private:
    
    T *elements; //pointer to dynamic array
    unsigned long long numElements;  //Current capacity
    unsigned long long numAllocated; //capacity of array
    static const unsigned long long DefaultNumberAllocated = 100;
    
public:
    
    //The maximum size of an IntVector.  This has to be constant
    //across all instances since we are using static memory to store
    //the backing array.  We declare CAPACITY as const to enforce this
    //and also as static, meaning that all IntVectors share the same
    //CAPACITY variable (i.e. the same memory storing the int).  We also
    //declare it as public, because users of our ADT may well want to know
    //the maximum capacity.
    //
    //This is the preferred approach.  (Don't even think about using a
    //global variable. But now you are thinking about it. Stop that.)
    static const unsigned long long CAPACITY = 200;
    
    //EFFECTS: Constructor
    Vector(unsigned long long capacity = DefaultNumberAllocated);
    
    //Custom Destructor;
    ~Vector();
    
    //Copy constructor
    Vector(const Vector<T> &other);
    
    //Assignment operator
    Vector<T> &operator=(const Vector<T> &rhs);
    
    void reserve();
    
    void resize();
    
    //REQUIRES: this IntVector is not full
    //MODIFIES: this IntVector
    //EFFECTS:  Adds value to the end of this IntVector.
    void push_back(T value);
    
    //REQUIRES: this IntVector is not empty
    //MODIFIES: this IntVector
    //EFFECTS:  Removes the last element in this IntVector.
    void pop_back();
    
    //REQUIRES: 0 <= index < number of elements in this IntVector
    //EFFECTS:  Returns (by reference) the element at the given index.
    unsigned long long &at(unsigned long long index);
    
    //REQUIRES: 0 <= index < number of elements in this IntVector
    //EFFECTS:  Returns (by reference) the element at the given index.
    const unsigned long long &at(unsigned long long index) const;
    
    //REQUIRES: 0 <= index < number of elements in this IntVector
    //EFFECTS:  Returns (by reference) the element at the given index.
    const T& operator[](unsigned long long index);
    
    //EFFECTS:  Returns the number of elements of this IntVector.
    unsigned long long size() const;
    
    unsigned long long capacity() const;
    
    //EFFECTS:  Returns true if this IntVector is empty, false otherwise.
    bool empty() const;
    
    //EFFECTS:  Returns true if this Vector is at capacity, false otherwise.
    //          That is, you may add elements if and only if full() is false.
    bool full() const;
};

//custom constructor
template<class T>
Vector<T>::Vector(unsigned long long capacity) : numElements(0), numAllocated(capacity) {
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
    for(unsigned long long i = 0; i < other.numElements; i++) {
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
    for(unsigned long long i = 0; i < rhs.numElements; i++) {
        elements[i] = rhs.elements[i];
    }
    return *this;
}

//Destructor
template<class T>
Vector<T>::~Vector() {
    delete[] elements;
}

//REQUIRES: this Vector is not full
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
const unsigned long long &Vector<T>::at(unsigned long long index) const {
    return elements[index];
}

//REQUIRES: 0 <= index < number of elements in this IntVector
//EFFECTS:  Returns (by reference) the element at the given index.
template<class T>
const T& Vector<T>::operator[] (unsigned long long index) {
    return elements[index];
}

//EFFECTS:  Returns the number of elements of this IntVector.
template<class T>
unsigned long long Vector<T>::size() const {
    return numElements;
}

//EFFECTS:  Returns the number of elements of this IntVector.
template<class T>
unsigned long long Vector<T>::capacity() const {
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
    reserve();
}

//needed for resize to size * 2
template<class T>
void Vector<T>::reserve() {
    T * tmp_array = new T[this->numAllocated];
    delete[] elements;
    numAllocated *= 2;
    for (unsigned long long i = 0; i < numElements; i++) {
        tmp_array[i] = elements[i];
    }
    elements = tmp_array;
}
