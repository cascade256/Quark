#pragma once
#include <cstring>
#include <cassert>

template <class T>
struct Array {
	T* data;
	int len;
	int capacity;
	T& operator[](int index) {
		return data[index];
	}
};

template <class T>
void arrayInit(Array<T>* arr) {
	arr->len = 0;
	arr->capacity = 1;
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
		delete[] oldData;
	}

	arr->data[arr->len] = item;
	arr->len++;
}

template <class T>
void arrayAdd(Array<T>* arr, Array<T>* items) {
	if (arr->capacity < arr->len + items->len) {
		T* oldData = arr->data;
		arr->capacity = arr->len + items->len;
		arr->data = new T[arr->capacity];
		memcpy(arr->data, oldData, sizeof(T) * arr->len);
		delete[] oldData;
	}
	memcpy(&arr->data[arr->len], items->data, sizeof(T) * items->len);
	arr->len += items->len;
}

template <class T>
void arrayAdd(Array<T>* arr, T* items, int len) {
	if (arr->capacity < arr->len + len) {
		T* oldData = arr->data;
		arr->capacity = arr->len + len;
		arr->data = new T[arr->capacity];
		memcpy(arr->data, oldData, sizeof(T) * arr->len);
		delete[] oldData;
	}
	memcpy(&arr->data[arr->len], items, sizeof(T) * len);
	arr->len += len;
}

template <class T>
void arrayInsert(Array<T>* arr, int i, T item) {
	assert(i <= arr->len);
	if (arr->len >= arr->capacity) {
		//We need to expand the array
		T* oldData = arr->data;
		arr->capacity *= 2;
		arr->data = new T[arr->capacity];
		memcpy(arr->data, oldData, sizeof(T) * arr->len);
		delete[] oldData;
	}
	memmove(&arr->data[i + 1], &arr->data[i], sizeof(T) * (arr->len - i));
	arr->data[i] = item;
	arr->len++;
}

template <class T>
void arrayInsert(Array<T>* arr, int i, Array<T>* items) {
	assert(i <= arr->len);
	if (arr->len >= arr->capacity) {
		//We need to expand the array
		T* oldData = arr->data;
		arr->capacity = arr->len + items->len;
		arr->data = new T[arr->capacity];
		memcpy(arr->data, oldData, sizeof(T) * arr->len);
		delete[] oldData;
	}
	//Move the data to make room for the inserted data
	memmove(&arr->data[i + items->len], &arr->data[i], sizeof(T) * (arr->len - i));
	
	//Insert the new data
	memcpy(&arr->data[i], items->data, sizeof(T) * items->len);
	arr->len += items->len;
}

template <class T>
void arrayInsert(Array<T>* arr, int i, const T* items, int numItems) {
	assert(i <= arr->len);
	if (arr->len >= arr->capacity) {
		//We need to expand the array
		T* oldData = arr->data;
		arr->capacity = arr->len + numItems;
		arr->data = new T[arr->capacity];
		memcpy(arr->data, oldData, sizeof(T) * arr->len);
		delete[] oldData;
	}
	//Move the data to make room for the inserted data
	memmove(&arr->data[i + numItems], &arr->data[i], sizeof(T) * (arr->len - i));

	//Insert the new data
	memcpy(&arr->data[i], items, sizeof(T) * numItems);
	arr->len += numItems;
}

template <class T>
int arrayIndexOf(Array<T>* arr, T item) {
	for (int i = 0; i < arr->len; i++) {
		if (arr->data[i] == item) {
			return i;
		}
	}
	return -1;
}

template <class T>
Array<T> arrayCopy(const Array<T>* arr) {
	Array<T> copy;
	copy.capacity = arr->capacity;
	copy.len = arr->len;
	copy.data = new T[copy.capacity];
	memcpy(copy.data, arr->data, copy.len * sizeof(T));
	return copy;
}

template <class T>
void arrayRemoveAt(Array<T>* arr, int i) {
	assert(i >= 0 && i < arr->len);
	memmove(&arr->data[i], &arr->data[i + 1], sizeof(T) * (arr->len - i));
	arr->len--;
}

//Removes the start item and up to, but not including, the end item
template <class T>
void arrayRemoveRange(Array<T>* arr, int start, int end) {
	assert(start >= 0 && start < arr->len);
	assert(end >= 0 && end <= arr->len);
	assert(start <= end);
	memmove(&arr->data[start], &arr->data[end], sizeof(T) * (arr->len - end));
	arr->len -= (end - start);
}


