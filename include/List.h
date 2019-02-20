#ifndef LIST_H
#define LIST_H

template <typename T>
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
	~List();
	List(const List&) = delete;
	void operator=(const List&) = delete;

	class Iterator {
	public:
		const baseType& operator[] (int index);
		bool operator!= (Iterator rhs);
		Iterator& operator++ ();
		Iterator operator++ (int);
		Iterator& operator-- ();
		Iterator operator-- (int);
	private:
		Iterator(Node<T>* node);
		Node<T>* node;
	};

	bool Empty();
	void AddToFront(T);
	void AddToBack(T);
	Iterator GetFront();
	Iterator GetBack();
	T&& RemoveFront();
	T&& RemoveBack();

private:
	Node<T> *front, *back;
};

#endif
