#include "Array.h"
/*
template <class T>
void arrayInit(Array<T>* arr) {
	arr->len = 0;
	arr->capacity = 0;
	arr->data = new T[arr->capacity];
}

template <class T>
void arrayAdd(Array<T>* arr, T item) {
	if (arr->len >= arr->capacity) {
		//We need to expand the array
		T* oldData = arr->data;
		arr->capacity *= 2;
		arr->data = new T[arr->capacity];
		memcpy(arr->data, oldData, sizeof(T) * arr->len);
	}

	arr->data[arr->len] = item;
	arr->len++;
}
*/