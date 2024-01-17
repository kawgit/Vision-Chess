#pragma once

#include <string>

#include "types.h"

Spiece char_to_spiece(char ch);
char spiece_to_char(Spiece spiece);

Spiece string_to_spiece(std::string str);
std::string spiece_to_string(Spiece spiece);

Square string_to_square(std::string str);
std::string square_to_string(Square square);

Square string_to_move(std::string str); // no flags save for promotion or castles
std::string move_to_string(Move move);
