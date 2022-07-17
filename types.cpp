#include <string>
#include <iostream>
#include <vector>
#include <cassert>
#include "types.h"

using namespace std;

const char piece_notations[] = {'p', 'n', 'b', 'r', 'q', 'k'};

string getSAN(Move m) {
    string res = square_to_string(get_from(m)) + square_to_string(get_to(m));

    if (is_promotion(m)) {
        assert(get_promotion_type(m) - PAWN >= 0);
        assert(get_promotion_type(m) - PAWN < 6);
        res += piece_notations[get_promotion_type(m) - PAWN];
    }

    return res;
}

string to_string(vector<Move> moves) {
	string result = "";
	for (Move& m : moves) {
        result += getSAN(m);
		result += " ";
    }
	return result;
}

void print(vector<Move> moves) {
    for (Move& m : moves) {
        cout<<getSAN(m)<<" ";
    }
}