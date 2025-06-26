// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "hwy/aligned_allocator.h"

template <typename T>
class AlignedArray
{
public:

	// *********************************************************************************************
	// Constructors
	AlignedArray() : allocationSize(0), numElements(0), data(nullptr) {}

	AlignedArray(int count) : allocationSize(count), numElements(0) {
		data = hwy::AllocateAligned<T>(count);
	}

	// *********************************************************************************************
	// Functions

	// Completely newly allocated array. Previous array is discarded.
	void Reallocate(int count) {
		data = hwy::AllocateAligned<T>(count);
		allocationSize = count;
	}

	// Allocates a new array and copies data from the original to the new array. 
	void Resize(int count) {
		auto temp = hwy::AllocateAligned<T>(count);

		if (data) {
			std::memcpy(temp.get(), data.get(), std::min(allocationSize, count) * sizeof(T));
		}

		data = std::move(temp);
		allocationSize = count;
	}

	// Ensures that the array is at least a certain size, and reallocates it if not. 
	// Whether or not the original array values are kept is determined by copyValues bool. 
	void EnsureSize(int count, bool copyValues = false) {

		if (allocationSize < count) {

			if (copyValues) {
				Resize(count);
			}
			else {
				Reallocate(count);
			}
		}
	}

	void Fill(T t) {
		std::fill(data.get(), data.get() + allocationSize, t);
	}

	// *********************************************************************************************
	// Getters

	T* GetPtr() {
		return data.get();
	}

	int GetSize() const {
		return allocationSize;
	}

	T& operator[](int index) {
		return data[index];
	}

	const T& operator[](int index) const {
		return data[index];
	}

	// *********************************************************************************************
	// Dynamic functions
	void Add(T t) {
		if (numElements == allocationSize) {
			Resize(allocationSize * 2);
		}

		GetPtr()[numElements] = t;
		++numElements;
	}

	T Get(int index) {
		if (index >= numElements || index < 0) {
			throw std::out_of_range("Index is out of bounds.");
		}
		return data[index];
	}

	void Set(int index, T t) {
		if (index >= numElements || index < 0) {
			throw std::out_of_range("Index is out of bounds.");
		}
		data[index] = t;
	}

	void Clear() {
		numElements = 0;
	}

	int Count() {
		return numElements;
	}

	// *********************************************************************************************
	// Data
private:
	int allocationSize;
	int numElements;
	hwy::AlignedFreeUniquePtr<T[]> data;
};