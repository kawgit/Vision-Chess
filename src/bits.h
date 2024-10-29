#pragma once

#include <cstdlib>
#include <stdint.h>
#include <vector>
#include <array>

#include "types.h"
#include "util.h"



extern int lookup67[68];

inline int lsb(const BB n) {
	return __builtin_ctzll(n);
}

inline void dellsb(BB& bb) {
	bb &= bb - 1;
}

inline int poplsb(BB& bb) {
	int i = lsb(bb);
	dellsb(bb);
	return i;
}

inline bool bb_has_multiple(BB bb) {
	dellsb(bb);
	return bool(bb);
}

inline int bitcount(BB x) {
	return __builtin_popcountll(x);
}

constexpr std::array<BB, 8> FILE_MASKS = []() constexpr {

	constexpr BB file1 = 0b0000000100000001000000010000000100000001000000010000000100000001;
    std::array<BB, 8> array {};

    for (size_t i = 0; i < 8; i++)

		array[i] = file1 << i;

    return array;

}();

constexpr std::array<BB, N_SQUARES + 1> SQUARE_MASKS = []() constexpr {

    std::array<BB, N_SQUARES + 1> array {};

	BB mask = 1Ull;

    for (size_t i = 0; i < N_SQUARES + 1; i++) {
		array[i] = mask;
		mask <<= 1;
	}

    return array;

}();

inline constexpr BB bb_of(Square square) {
	return SQUARE_MASKS[square];
}

inline constexpr BB bb_of_file(File file) {
	return FILE_MASKS[file];
}

constexpr std::array<BB, 8> RANK_MASKS = []() constexpr {

	constexpr BB rank1 = 0b11111111;
    std::array<BB, 8> array {};

    for (size_t i = 0; i < 8; i++)

		array[i] = rank1 << (i * 8);

    return array;

}();

inline constexpr BB bb_of_rank(Rank rank) {
	return RANK_MASKS[rank];
}

inline constexpr bool bb_has(BB bb, Square square) {
	return bb & bb_of(square);
}

inline BB rand64() {
	return ((BB)rand() << 48) | ((BB)rand() << 32) | ((BB)rand() << 16) | (BB)rand();
}

inline float randf(float lb, float ub) {
	return lb + (float)rand() / (float)RAND_MAX * (float)(ub-lb);
}

template <Direction D> BB shift(BB a);

template<> inline constexpr BB shift<NORTH>(BB a) {
    return 	a 						 << 8;
}

template<> inline constexpr BB shift<NORTHEAST>(BB a) {
    return (a & ~bb_of_file(FILE_H)) << 9;
}

template<> inline constexpr BB shift<EAST>(BB a) {
    return (a & ~bb_of_file(FILE_H)) << 1;
}

template<> inline constexpr BB shift<SOUTHEAST>(BB a) {
    return (a & ~bb_of_file(FILE_H)) >> 7;
}

template<> inline constexpr BB shift<SOUTH>(BB a) {
    return a 						 >> 8;
}

template<> inline constexpr BB shift<SOUTHWEST>(BB a) {
    return (a & ~bb_of_file(FILE_A)) >> 9;
}

template<> inline constexpr BB shift<WEST>(BB a) {
    return (a & ~bb_of_file(FILE_A)) >> 1;
}

template<> inline constexpr BB shift<NORTHWEST>(BB a) {
    return (a & ~bb_of_file(FILE_A)) << 7;
}

void print(BB bb);

constexpr std::array<std::array<BB, N_DIRECTIONS>, N_SQUARES> gun_bbs = []() constexpr {
	
	std::array<std::array<BB, N_DIRECTIONS>, N_SQUARES> result { BB_EMPTY };
	
	for (Square square = A1; square <= H8; square++) {
		
		const Rank rank = rank_of(square);
		const File file = file_of(square);

		for (Direction direction = NORTH; direction <= NORTHWEST; direction++) {
			
			for (Square magnitude = 1; magnitude < N_FILES; magnitude++) {

				const Rank target_rank = rank + DIRECTION_OFFSETS[direction][1] * magnitude;
				const File target_file = file + DIRECTION_OFFSETS[direction][0] * magnitude;

				if (target_rank < RANK_1 || target_rank > RANK_8 || target_file < FILE_A || target_file > FILE_H)
					break;

				const Square target_square = square_of(target_rank, target_file);

				result[square][direction] |= bb_of(target_square);

			}

		}
	}

	return result;

}();

inline constexpr BB bb_gun(const Square from_square, const Direction direction) {
	return gun_bbs[from_square][direction];
}

constexpr std::array<std::array<BB, N_SQUARES>, N_SQUARES> segment_bbs = []() constexpr {
	
	std::array<std::array<BB, N_SQUARES>, N_SQUARES> result { BB_EMPTY };
	
	for (Square from_square = A1; from_square <= H8; from_square++) {
		for (Square to_square = A1; to_square <= H8; to_square++) {
		
			const Rank from_rank = rank_of(from_square);
			const File from_file = file_of(from_square);

			for (Direction direction = NORTH; direction <= NORTHWEST; direction++) {
				
				if (!bb_has(bb_gun(from_square, direction), to_square))
					continue;

				for (size_t magnitude = 1; magnitude < N_FILES; magnitude++) {

					const Rank target_rank = from_rank + DIRECTION_OFFSETS[direction][1] * magnitude;
					const File target_file = from_file + DIRECTION_OFFSETS[direction][0] * magnitude;

					if (target_rank < RANK_1 || target_rank > RANK_8 || target_file < FILE_A || target_file > FILE_H)
						break;

					const Square target_square = square_of(target_rank, target_file);

					if (target_square == to_square) 
						break;

					result[from_square][to_square] |= bb_of(target_square);

				}
			}
		}
	}

	return result;

}();

inline constexpr BB bb_segment(const Square from_square, const Square to_square) {
	return segment_bbs[from_square][to_square];
}

constexpr std::array<std::array<BB, N_SQUARES>, N_SQUARES> ray_bbs = []() constexpr {
	
	std::array<std::array<BB, N_SQUARES>, N_SQUARES> result { BB_EMPTY };
	
	for (Square from_square = A1; from_square <= H8; from_square++) {
		for (Square to_square = A1; to_square <= H8; to_square++) {
		
			const Rank from_rank = rank_of(from_square);
			const File from_file = file_of(from_square);

			for (Direction direction = NORTH; direction <= NORTHWEST; direction++) {
				
				if (!bb_has(bb_gun(from_square, direction), to_square))
					continue;

				for (size_t magnitude = 1; magnitude < N_FILES; magnitude++) {

					const Rank target_rank = from_rank + DIRECTION_OFFSETS[direction][1] * magnitude;
					const File target_file = from_file + DIRECTION_OFFSETS[direction][0] * magnitude;

					if (target_rank < RANK_1 || target_rank > RANK_8 || target_file < FILE_A || target_file > FILE_H)
						break;

					const Square target_square = square_of(target_rank, target_file);

					result[from_square][to_square] |= bb_of(target_square);

				}
			}
		}
	}

	return result;

}();

inline constexpr BB bb_ray(const Square from_square, const Square to_square) {
	return ray_bbs[from_square][to_square];
}