#include "pos.h"
#include "bits.h"
#include <stdint.h>
#include <iostream>
#include <cstdlib>

using namespace std;



int lsb(BB& n) {
	const static int lookup67[68] = {
		64,  0,  1, 39,  2, 15, 40, 23,
		3 , 12, 16, 59, 41, 19, 24, 54,
		4 , -1, 13, 10, 17, 62, 60, 28,
		42, 30, 20, 51, 25, 44, 55, 47,
		5 , 32, -1, 38, 14, 22, 11, 58,
		18, 53, 63,  9, 61, 27, 29, 50,
		43, 46, 31, 37, 21, 57, 52,  8,
		26, 49, 45, 36, 56,  7, 48, 35,
		6 , 34, 33, -1 };
	return lookup67[(n & -n) % 67];
}

int poplsb(BB& n) {
	int i = lsb(n);
	n &= n - 1;
	return i;
}

const uint64_t m1  = 0x5555555555555555; //binary: 0101...
const uint64_t m2  = 0x3333333333333333; //binary: 00110011..
const uint64_t m4  = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
const uint64_t h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...

int bitcount(BB x)
{
    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
    return (x * h01) >> 56;  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 
}

bool bitAt(BB& n, int k) {
	return n & (1ULL << k);
}

int rc(int r, int c) {
	return r*8+c;
}

void print(BB n) {
	for (int r = 7; r >= 0; r--) {
		for (int c = 0; c < 8; c++) {
			cout<<(bitAt(n, rc(r, c)) ? "X" : "-")<<" ";
		}
		cout<<endl;
	}
	cout<<endl;
}

BB row1 = 0b11111111;
BB col1 = 0b0000000100000001000000010000000100000001000000010000000100000001;

BB getColMask(int c) {
	return col1<<c;
}

BB getRowMask(int r) {
	return row1<<(r*8);
}

BB getBB(int s) {
	return 1ULL<<s;
}

BB randBB() {
	return ((BB)rand() << 48) | ((BB)rand() << 32) | ((BB)rand() << 16) | (BB)rand();
}