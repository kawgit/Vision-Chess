#pragma once

#include <string>

#include "types.h"

#include <unistd.h>
#define sleep_ms(ms) usleep(ms * 1000)

#define handled_assert(assertion) {                                                                                       \
    if (!(assertion)) {                                                                                               \
        std::cout << "info string assertion \"" << #assertion << "\" failed at " << __FILE__ << ":" << __LINE__ << std::endl; \
        goto handled_assert_out;                                                                                           \
    }                                                                                                                 \
}

inline constexpr Square square_of(const Rank rank, const File file) {
    return rank * N_FILES + file;
}

inline constexpr Rank rank_of(const Square square) {
    return square / N_FILES;
}

inline constexpr File file_of(const Square square) {
    return square % N_FILES;
}

inline constexpr Square flip_rank(const Square square) {
    return square ^ 0b111000;
}

inline constexpr Square flip_file(const Square square) {
    return square ^ 0b000111;
}

inline constexpr Square flip_components(const Square square, const bool rank, const bool file) {
    return square ^ (0b111000 * rank | 0b000111 * file);
}


Spiece char_to_spiece(char ch);
char spiece_to_char(Spiece spiece);

Spiece string_to_spiece(std::string str);
std::string spiece_to_string(Spiece spiece);

Square string_to_square(std::string str);
std::string square_to_string(Square square);

Square string_to_move(std::string str); // no flags save for promotion or castles
std::string move_to_string(const Move move);

std::string movelist_to_string(const std::vector<Move> moves);

std::vector<std::string> split(const std::string& str);