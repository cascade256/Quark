#pragma once

template <class T>
struct Array {
	T* data;
	int len;
	int capacity;
};

/*template <class T>
void arrayInit(Array<T>* arr);

template <class T>
void arrayAdd(Array<T>* arr, T item);
*/
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
void arrayRemoveAt(Array<T>* arr, int i) {
	memcpy(&arr->data[i], &arr->data[i + 1], sizeof(T) * (arr->len - i));
	arr->len--;
}


