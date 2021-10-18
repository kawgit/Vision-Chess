#include "movegen.h"
#include "pos.h"
#include "util.h"
#include "types.h"
#include <iostream>
#include <vector>

using namespace std;

BB bishop_magics[64];
BB rook_magics[64];
BB bishop_table[64][1<<BISHOP_BITS];
BB rook_table[64][1<<ROOK_BITS];

BB rays[64][8];

BB rook_moves[64];
BB bishop_moves[64];

BB rook_blockermasks[64];
BB bishop_blockermasks[64];

int directions[8][2] = {{0, 1},{1, 1},{1, 0},{1, -1},{0, -1},{-1, -1},{-1, 0},{-1, 1}};

BB getAnswer(int r, int c, BB blockerboard, int d_offset) {
    BB answer = 0;
    for (int d = d_offset; d < 8; d+=2) {
        for (int m = 1; m <= 7; m++) {
            int _r = r + directions[d][1]*m;
            int _c = c + directions[d][0]*m;
            int _s = RC2SQ(_r, _c);
            if (_r >= 0 && _r <= 7 && _c >= 0 && _c <= 7) {
                answer |= getBB(_s);
                if (bitAt(blockerboard, _s)) break;
            }
            else break;
        }
    }
    return answer;
}

void initM(int seed) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            for (int d = 0; d < 8; d++) {
                for (int m = 1; m <= 7; m++) {
                    int _r = r + directions[d][1]*m;
                    int _c = c + directions[d][0]*m;
                    if (_r >= 0 && _r <= 7 && _c >= 0 && _c <= 7) {
                        rays[RC2SQ(r, c)][d] |= getBB(RC2SQ(_r, _c));
                    }
                }
            }
        }
    }

    for (int s = 0; s < 64; s++) {
        rook_moves[s] = rays[s][NORTH] | rays[s][EAST] | rays[s][SOUTH] | rays[s][WEST];
        bishop_moves[s] = rays[s][NORTHEAST] | rays[s][SOUTHEAST] | rays[s][SOUTHWEST] | rays[s][NORTHWEST];
    }

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            BB exclude = 0ULL;
            if (c != 0) exclude |= file_masks[0];
            if (c != 7) exclude |= file_masks[7];
            if (r != 0) exclude |= rank_masks[0];
            if (r != 7) exclude |= rank_masks[7];

            int s = RC2SQ(r, c);
            rook_blockermasks[s] = rook_moves[s] & ~exclude;
            bishop_blockermasks[s] = bishop_moves[s] & ~exclude;
        }
    }

    vector<BB> rook_blockerboards[64];
    vector<BB> bishop_blockerboards[64];
    for (int s = 0; s < 64; s++) {
        {
            int count = bitcount(rook_blockermasks[s]);
            for (uint32_t bits = 0; bits < (1ULL<<count); bits++) {
                BB save = rook_blockermasks[s];
                rook_blockerboards[s].push_back(rook_blockermasks[s]);
                for (int i = 0; i < count; i++)
                    if (!bitAt(bits, i)) rook_blockerboards[s][bits] &= ~getBB(poplsb(save));
                    else poplsb(save);
            }
        }

        {        
            int count = bitcount(bishop_blockermasks[s]);
            for (uint32_t bits = 0; bits < (1ULL<<count); bits++) {
                BB save = bishop_blockermasks[s];
                bishop_blockerboards[s].push_back(bishop_blockermasks[s]);
                for (int i = 0; i < count; i++)
                    if (!bitAt(bits, i)) bishop_blockerboards[s][bits] &= ~getBB(poplsb(save));
                    else poplsb(save);
            }
        }
    }

    srand(seed);
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int s = RC2SQ(r, c);
            bool found = false;
            while (!found) {
                found = true;
                BB magic = randBB();

                for (BB blockerboard : bishop_blockerboards[s]) {
                    BB answer = getAnswer(r, c, blockerboard, NORTHEAST);
                    BB index = (blockerboard*magic)>>(BISHOP_SHIFT);

                    if (bishop_table[s][index] == 0) bishop_table[s][index] = answer;
                    else if (bishop_table[s][index] != answer) {
                        found = false;
                        for (int i = 0; i < (1<<BISHOP_BITS); i++) 
                            bishop_table[s][i] = 0;
                        break;
                    }
                }

                if (found)
                    bishop_magics[s] = magic;
            }
        }
    }
}