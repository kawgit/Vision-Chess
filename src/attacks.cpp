#include <vector>
#include <array>
#include <cassert>
#include <iostream>

#include "attacks.h"
#include "bits.h"
#include "types.h"
#include "util.h"



namespace attacks {

    BB bishop_blockermasks [N_SQUARES];
    BB rook_blockermasks   [N_SQUARES];

    BB bishop_magics [N_SQUARES];
    BB rook_magics   [N_SQUARES];
    
    std::array<std::array<BB, MAGIC_ENTRY_SIZE>, N_SQUARES> bishop_table;
    std::array<std::array<BB, MAGIC_ENTRY_SIZE>, N_SQUARES> rook_table;

    std::array<Square, 2> orthogonal_offsets[N_DIRECTIONS / 2] = {{ 0,  1}, { 1,  0}, { 0, -1}, {-1,  0}};
    std::array<Square, 2> diagonal_offsets  [N_DIRECTIONS / 2] = {{ 1,  1}, { 1, -1}, {-1, -1}, {-1,  1}};

    BB generate_attacks(const Square square, BB blockerboard, std::array<Square, 2>* direction_offsets, size_t n_directions) {
        BB attacks_bb = BB_EMPTY;
        
        Rank rank = rank_of(square);
        File file = file_of(square);

        for (size_t d = 0; d < n_directions; d++) {

            for (size_t m = 1; m <= 7; m++) {

                Rank target_rank = rank + direction_offsets[d][1] * m;
                File target_file = file + direction_offsets[d][0] * m;

                if (target_rank < RANK_1 || target_rank > RANK_8 || target_file < FILE_A || target_file > FILE_H)
                    break;
                
                Square target_square = square_of(target_rank, target_file);

                attacks_bb |= bb_of(target_square);

                if (bb_has(blockerboard, target_square))
                    break;

            }

        }

        return attacks_bb;
    }
    
    void generate_blockerboards(std::vector<BB>* blockerboards, BB* blockermasks) {

        for (Square square = A1; square <= H8; square++) {

            int count = bitcount(blockermasks[square]);

            for (BB bits = BB_EMPTY; bits < (1ULL << count); bits++) {

                BB target_squares = blockermasks[square];
                BB blockerboard = target_squares;
                
                for (int i = 0; i < count; i++) {
                    
                    Square target_square = poplsb(target_squares);

                    blockerboard &= bb_has(bits, i) ? BB_FULL : ~bb_of(target_square);

                }

                blockerboards[square].push_back(blockerboard);
            }
        }
    }
        
    void generate_solutions(std::vector<BB>* solutions, std::vector<BB>* blockerboards, std::array<Square, 2>* direction_offsets, size_t n_directions) {

        for (Square square = A1; square <= H8; square++) {

            for (BB blockerboard : blockerboards[square]) {

                BB solution = generate_attacks(square, blockerboard, direction_offsets, n_directions);

                solutions[square].push_back(solution);

            }
        }
    }

    void generate_magics(BB* magics, std::array<std::array<BB, MAGIC_ENTRY_SIZE>, N_SQUARES>& table, const std::vector<BB>* blockerboards, const std::vector<BB>* solutions) {

        for (Square square = A1; square <= H8; square++) {

            bool found_magic = false;

            while (!found_magic) {

                found_magic = true;
                
                magics[square] = rand_BB();

                for (size_t scenario = 0; scenario < blockerboards[square].size(); scenario++) {
                    BB blockerboard = blockerboards[square][scenario];
                    BB solution     = solutions    [square][scenario];

                    BB index = (blockerboard * magics[square]) >> MAGIC_SHIFT;

                    if (table[square][index] == 0)
                        table[square][index] = solution;
                    else if (table[square][index] != solution) {
                        for (size_t i = 0; i < MAGIC_ENTRY_SIZE; i++) 
                            table[square][i] = 0;
                        found_magic = false;     
                        break;
                    }
                }
            }

        }
    }

    void init() {

        // Generate blockermasks

        for (Square square = A1; square <= H8; square++) {
            Rank rank = rank_of(square);
            Rank file = file_of(square);

            BB exclude = 0ULL;

            if (file != FILE_A) exclude |= bb_of_file(FILE_A);
            if (file != FILE_H) exclude |= bb_of_file(FILE_H);
            if (rank != RANK_1) exclude |= bb_of_rank(RANK_1);
            if (rank != RANK_8) exclude |= bb_of_rank(RANK_8);

            bishop_blockermasks[square] = bishop_atks[square] & ~exclude;
            rook_blockermasks  [square] = rook_atks  [square] & ~exclude;
        }

        // Generate blockerboards

        std::vector<BB> bishop_blockerboards[N_SQUARES];
        std::vector<BB> rook_blockerboards  [N_SQUARES];

        generate_blockerboards(bishop_blockerboards, bishop_blockermasks);
        generate_blockerboards(rook_blockerboards,   rook_blockermasks);
        
        // Generate solutions

        std::vector<BB> bishop_solutions[N_SQUARES];
        std::vector<BB> rook_solutions  [N_SQUARES];

        generate_solutions(bishop_solutions, bishop_blockerboards, diagonal_offsets,   4);
        generate_solutions(rook_solutions,   rook_blockerboards,   orthogonal_offsets, 4);

        // Generate magics

        generate_magics(bishop_magics, bishop_table, bishop_blockerboards, bishop_solutions);
        generate_magics(rook_magics,   rook_table,   rook_blockerboards,   rook_solutions  );
        
    }

}