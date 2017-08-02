#pragma once

template <class T>
struct Array {
	T* data;
	int len;
	int capacity;
	T& operator[](int index) {
		return data[index];
	}
};

/*template <class T>
void arrayInit(Array<T>* arr);

template <class T>
void arrayAdd(Array<T>* arr, T item);
*/
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
	}
	memcpy(&arr->data[i + 1], &arr->data[i], sizeof(T) * (arr->len - i));
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
	}
	//Move the data to make room for the inserted data
	memcpy(&arr->data[i + items->len], &oldData[i], sizeof(T) * (arr->len - i));
	
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
	}
	//Move the data to make room for the inserted data
	memcpy(&arr->data[i + numItems], &arr->data[i], sizeof(T) * (arr->len - i));

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
	memcpy(&arr->data[i], &arr->data[i + 1], sizeof(T) * (arr->len - i));
	arr->len--;
}

