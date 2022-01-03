#include <string>
#include <iostream>
#include <vector>
#include "types.h"

using namespace std;

string getSAN(Move m) {
    const static string piece_notations[] = {"p", "n", "b", "r", "q", "k"};
    return sqToNot(getFrom(m)) + sqToNot(getTo(m)) + (isPromotion(m) ? piece_notations[getPromotionType(m)] : "");
}

void print(vector<Move> moves) {
    for (Move& m : moves) {
        cout<<getSAN(m)<<" ";
    }
}