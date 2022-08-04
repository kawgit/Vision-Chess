#include "pos.h"
#include "bits.h"
#include <stdint.h>
#include <iostream>
#include <cstdlib>

using namespace std;

int lookup67[68] = {
	64,  0,  1, 39,  2, 15, 40, 23,
	3 , 12, 16, 59, 41, 19, 24, 54,
	4 , -1, 13, 10, 17, 62, 60, 28,
	42, 30, 20, 51, 25, 44, 55, 47,
	5 , 32, -1, 38, 14, 22, 11, 58,
	18, 53, 63,  9, 61, 27, 29, 50,
	43, 46, 31, 37, 21, 57, 52,  8,
	26, 49, 45, 36, 56,  7, 48, 35,
	6 , 34, 33, -1 };

void print(BB n) {
	for (int r = 7; r >= 0; r--) {
		for (int c = 0; c < 8; c++) {
			cout<<(bitAt(n, rc(r, c)) ? "X" : "-")<<" ";
		}
		cout<<endl;
	}
	cout<<endl;
}

vector<float> serialize_bb(BB bb) {
	vector<float> vec;
	for (int i = 0; i < bb; i++) {
		vec.emplace_back(bitAt(bb, i) ? 1 : 0);
	}
	return vec;
}

void add_serialized_bb(vector<float>& vec, BB bb) {
	for (int i = 0; i < bb; i++) {
		vec.emplace_back(bitAt(bb, i) ? 1 : 0);
	}
}