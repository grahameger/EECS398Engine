#include <utility>
#include "List.h"

template <typename T>
struct Node {
	Node(T d, Node* p, Node* n) : data(d), prev(p), next(n) {}

	T data;
	Node* prev;
	Node* next;
};

template <typename T>
List<T>::List() : front(nullptr), back(nullptr) {}

template <typename T>
List<T>::~List() {
	while(back != nullptr) {
		Node<T>* nextBack = back->prev;
		delete back;
		back = nextBack;
	}
}

template <typename T>
List<T>::Iterator::Iterator(Node<T>* n) : node(n) {}

template <typename T>
const typename List<T>::baseType& List<T>::Iterator::operator[] (int index) {
	return node->data[index];
}

template <typename T>
bool List<T>::Iterator::operator != (Iterator rhs) {
	return node == rhs.node;
}

template <typename T>
typename List<T>::Iterator& List<T>::Iterator::operator++ () {
	node = node->next;
	return *this;
}

template <typename T>
typename List<T>::Iterator List<T>::Iterator::operator++ (int i) {
	Node<T>* curNode = node;
	node = node->next;
	return Iterator(curNode);
}

template <typename T>
typename List<T>::Iterator& List<T>::Iterator::operator-- () {
	node = node->prev;
	return *this;
}

template <typename T>
typename List<T>::Iterator List<T>::Iterator::operator-- (int i) {
	Node<T>* curNode = node;
	node = node->prev;
	return Iterator(curNode);
}

template <typename T>
bool List<T>::Empty() { return front == nullptr; }

template <typename T>
void List<T>::AddToFront(T toAdd) {
	Node<T>* newNode = new Node<T>(toAdd, nullptr, front);
	front->prev = newNode;

	front = newNode;
}

template <typename T>
void List<T>::AddToBack(T toAdd) {
	Node<T>* newNode = new Node<T>(toAdd, back, nullptr);
	back->next = newNode;

	back = newNode;
}

template <typename T>
typename List<T>::Iterator List<T>::GetFront() {
	return Iterator(front);
}

template <typename T>
typename List<T>::Iterator List<T>::GetBack() {
	return Iterator(back);
}

template <typename T>
T&& List<T>::RemoveFront() {
	T returnVal = std::move(front.data);

	front = front.next;
	delete front.prev;
	front.prev = nullptr;

	return returnVal;
}

template <typename T>
T&& List<T>::RemoveBack() {
	T returnVal = std::move(back.data);

	back = back.prev;
	delete back.next;
	back.next = nullptr;

	return returnVal;
}
