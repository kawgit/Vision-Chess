#include <string>
#include <iostream>
#include <vector>
#include "types.h"

using namespace std;

string getSAN(Move m) {
    const static string piece_notations[] = {"p", "n", "b", "r", "q", "k"};
    return square_to_string(get_from(m)) + square_to_string(get_to(m)) + (is_promotion(m) ? piece_notations[get_promotion_type(m)] : "");
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