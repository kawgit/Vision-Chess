#pragma once

#include <cstdlib>
#include <stdint.h>
#include <vector>

using namespace std;

typedef uint64_t BB;

extern int lookup67[68];

inline int lsb(BB& n) {
	return lookup67[(n & -n) % 67];
}

inline int poplsb(BB& n) {
	int i = lsb(n);
	n &= n - 1;
	return i;
}

const uint64_t m1  = 0x5555555555555555; //binary: 0101...
const uint64_t m2  = 0x3333333333333333; //binary: 00110011..
const uint64_t m4  = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
const uint64_t h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...

inline int bitcount(BB x)
{
    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
    return (x * h01) >> 56;  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
}

inline bool bitAt(BB& n, int k) {
	return n & (1ULL << k);
}

inline int rc(int r, int c) {
	return r*8+c;
}

void print(BB n);

const BB row1 = 0b11111111;
const BB col1 = 0b0000000100000001000000010000000100000001000000010000000100000001;

inline BB get_file_mask(int c) {
	return col1<<c;
}

inline BB get_rank_mask(int r) {
	return row1<<(r*8);
}

inline BB get_BB(int s) {
	return 1ULL<<s;
}

inline BB rand_BB() {
	return ((BB)rand() << 48) | ((BB)rand() << 32) | ((BB)rand() << 16) | (BB)rand();
}

inline float randf(float lb, float ub) {
	return lb + (float)rand() / (float)RAND_MAX * (float)(ub-lb);
}

vector<float> serialize_bb(BB bb);

void add_serialized_bb(vector<float>& vec, BB bb);