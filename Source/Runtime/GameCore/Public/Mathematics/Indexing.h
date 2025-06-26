// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

inline constexpr int Length(const int sizeX, const int sizeY) {
	return sizeX * sizeY;
}

inline constexpr int Length(const int sizeX, const int sizeY, const int sizeZ) {
	return sizeX * sizeY * sizeZ;
}

inline constexpr int Flatten(const int x, const int y, const int sizeX) {
	return (y * sizeX) + x;
}

inline constexpr int Flatten(
	const int x, const int y, const int z, 
	const int sizeX, const int sizeY) {
	return (z * sizeX * sizeY) + (y * sizeX) + x;
}

inline constexpr void Unravel(const int index, const int sizeX, int& x, int& y) {
	y = index / sizeX;
	x = index % sizeX;
}

inline constexpr void Unravel(
	const int index, 
	const int sizeX, const int sizeY, 
	int& x, int& y, int& z) {
	int sizeXY = sizeX * sizeY;
	int countTop = index % sizeXY; // Number of elements in the max Z layer
	z = index / sizeXY;
	y = countTop / sizeX;
	x = countTop % sizeX;
}

// Equivalent to taking a flattened i index, unravelling it, adding some offset, and then flattening
// again. 
inline constexpr int Adjust(const int index, const int sizeX, int x, int y) {
	return index + Flatten(x, y, sizeX);
}

inline constexpr int Adjust(
	const int index, const int sizeX, const int sizeY,
	const int x, const int y, const int z
) {
	return index + Flatten(x, y, z, sizeX, sizeY);
}

// Returns the size of the padding required to have count be a multiple of chunk. 
inline constexpr int GetPadding(int count, int chunk) {
	int pad = (chunk - (count % chunk));
	if (pad == chunk) return 0;
	return pad;
}