#include <string>
#include <map>

#include "types.h"
#include "util.h"
#include "move.h"

using namespace std;


std::map<char, Spiece> char_to_spiece_map = {
    {'p', BLACK_PAWN},
    {'n', BLACK_KNIGHT},
    {'b', BLACK_BISHOP},
    {'r', BLACK_ROOK},
    {'q', BLACK_QUEEN},
    {'k', BLACK_KING},
    {'P', WHITE_PAWN},
    {'N', WHITE_KNIGHT},
    {'B', WHITE_BISHOP},
    {'R', WHITE_ROOK},
    {'Q', WHITE_QUEEN},
    {'K', WHITE_KING},
};

Spiece char_to_spiece(char ch) {
    return char_to_spiece_map[ch];
}

std::map<Spiece, char> spiece_to_char_map = {
    {SPIECE_NONE, ' '},
    {BLACK_PAWN, 'p'},
    {BLACK_KNIGHT, 'n'},
    {BLACK_BISHOP, 'b'},
    {BLACK_ROOK, 'r'},
    {BLACK_QUEEN, 'q'},
    {BLACK_KING, 'k'},
    {WHITE_PAWN, 'P'},
    {WHITE_KNIGHT, 'N'},
    {WHITE_BISHOP, 'B'},
    {WHITE_ROOK, 'R'},
    {WHITE_QUEEN, 'Q'},
    {WHITE_KING, 'K'},
};

char spiece_to_char(Spiece spiece) {
    return spiece_to_char_map[spiece];
}

Spiece string_to_spiece(string str) {
    return char_to_spiece(str[0]);
}

string spiece_to_string(Spiece spiece) {
    return string(1, spiece_to_char(spiece));
}

Square string_to_square(string str) {
    return square_of(str[1] - '1', str[0] - 'a');
}

string square_to_string(Square square) {
    return string(1, file_of(square) + 'a') + string(1, rank_of(square) + '1');
}

std::map<char, MoveFlag> char_to_promotion_flag = {
    {'n', N_PROM},
    {'b', B_PROM},
    {'r', R_PROM},
    {'q', Q_PROM},
};

Square string_to_move(string str) {
    Square from_square = string_to_square(str.substr(0, 2));
    Square to_square = string_to_square(str.substr(2, 2));
    MoveFlag flags = str.size() == 6 ? char_to_promotion_flag[str[5]] : QUIET;
    return make_move(from_square, to_square, flags);
}

std::map<Piece, char> piece_to_char_map = {
    {PAWN, 'p'},
    {KNIGHT, 'n'},
    {BISHOP, 'b'},
    {ROOK, 'r'},
    {QUEEN, 'q'},
    {KING, 'k'},
};

string move_to_string(Move move) {
    return square_to_string(move::from_square(move)) + square_to_string(move::to_square(move)) + (move::is_promotion(move) ? "=" + string(1, piece_to_char_map[move::promotion_piece(move)]) : "");
}
