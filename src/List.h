#ifndef LIST_H
#define LIST_H

class Node;

template <typename T>
struct PointerLess { typedef T type; };
template <typename T>
struct PointerLess<T*> { typedef T type; };

template <typename T>
class List {
public:
	typedef typename PointerLess<T>::type baseType;

	List();

	class Iterator {
	public:
		const baseType& operator[] (int index);
		bool operator!= (Iterator rhs);
		Iterator& operator++ ();
		Iterator operator++ (int);
		Iterator& operator-- ();
		Iterator operator-- (int);
	private:
		Node* node;
	};

	bool Empty();
	void AddToFront(T);
	void AddToBack(T);
	Iterator GetFront();
	Iterator GetBack();
	T RemoveFront();
	T RemoveBack();

private:
	Node *front, *back;
	unsigned int size;
};

#endif
