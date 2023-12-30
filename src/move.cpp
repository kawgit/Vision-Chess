#include "move.h"

const char piece_notations[] = {'p', 'n', 'b', 'r', 'q', 'k'};

string to_san(Move m) {
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
        result += to_san(m);
		result += " ";
    }
	return result;
}

void print(vector<Move> moves) {
    for (Move& m : moves) {
        cout << to_san(m) << " ";
    }
    cout << endl;
}