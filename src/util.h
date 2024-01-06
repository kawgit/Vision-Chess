#pragma once

#include <string>

#include "types.h"
#include "move.h"

using namespace std;

constexpr inline bool is_okay_color (const Color color)   { return color == WHITE || color == BLACK; }
constexpr inline bool is_okay_piece (const Piece piece)   { return piece >= PAWN && piece <= KING; }
constexpr inline bool is_okay_square(const Square square) { return square >= A1 && square <= H8; }

constexpr inline Square square_of(const Rank rank, const File file) { return rank * N_FILES + file; }
constexpr inline Rank rank_of(const Square sq) { return sq / 8; }
constexpr inline File file_of(const Square sq) { return sq % 8; }

Spiece char_to_spiece(char ch);
char spiece_to_char(Spiece spiece);

Spiece string_to_spiece(string str);
string spiece_to_string(Spiece spiece);

Square string_to_square(string str);
string square_to_string(Square square);

Square string_to_move(string str); // no flags save for promotion or castles
string move_to_string(Move move);
