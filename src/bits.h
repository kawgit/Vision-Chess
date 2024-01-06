#pragma once

#include <cstdlib>
#include <stdint.h>
#include <vector>
#include <array>

#include "types.h"

using namespace std;

extern int lookup67[68];

inline int lsb(BB& n) {
	#ifdef DONT_USE_BUILTINS
		return lookup67[(n & -n) % 67];
	#else
		return __builtin_ctzll(n);
	#endif
}

inline int poplsb(BB& n) {
	int i = lsb(n);
	n &= n - 1;
	return i;
}

inline int bitcount(BB x) {
	#ifdef DONT_USE_BUILTINS
		const uint64_t m1  = 0x5555555555555555; //binary: 0101...
		const uint64_t m2  = 0x3333333333333333; //binary: 00110011..
		const uint64_t m4  = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
		const uint64_t h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...
		
		x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
		x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
		x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
		return (x * h01) >> 56;  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
	#else
		return __builtin_popcountll(x);
	#endif
}

constexpr std::array<BB, 8> FILE_MASKS = []() constexpr {

	constexpr BB file1 = 0b0000000100000001000000010000000100000001000000010000000100000001;
    std::array<BB, 8> array {};

    for (size_t i = 0; i < 8; i++)

		array[i] = file1 << i;

    return array;

}();

constexpr std::array<BB, 64> SQUARE_MASKS = []() constexpr {

    std::array<BB, 64> array {};

    for (size_t i = 0; i < 64; i++)

		array[i] = 1ULL << i;

    return array;

}();

constexpr inline BB bb_of(Square square) {
	return SQUARE_MASKS[square];
}

constexpr inline BB bb_of_file(File file) {
	return FILE_MASKS[file];
}

constexpr std::array<BB, 8> RANK_MASKS = []() constexpr {

	constexpr BB rank1 = 0b11111111;
    std::array<BB, 8> array {};

    for (size_t i = 0; i < 8; i++)

		array[i] = rank1 << (i * 8);

    return array;

}();

constexpr inline BB bb_of_rank(Rank rank) {
	return RANK_MASKS[rank];
}

constexpr inline bool bb_has(BB bb, Square square) {
	return bb & bb_of(square);
}

inline BB rand_BB() {
	return ((BB)rand() << 48) | ((BB)rand() << 32) | ((BB)rand() << 16) | (BB)rand();
}

inline float randf(float lb, float ub) {
	return lb + (float)rand() / (float)RAND_MAX * (float)(ub-lb);
}

template <Directions D> BB shift(BB a);

template<> constexpr inline BB shift<NORTH>(BB a) {
    return a << 8;
}

template<> constexpr inline BB shift<SOUTH>(BB a) {
    return a >> 8;
}

template<> constexpr inline BB shift<EAST>(BB a) {
    return (a & ~bb_of_file(7)) << 1;
}

template<> constexpr inline BB shift<WEST>(BB a) {
    return (a & ~bb_of_file(0)) >> 1;
}

void print(BB bb);
