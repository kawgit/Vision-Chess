#pragma once

#include "pos.h"
#include "types.h"
#include "bits.h"
#include <vector>

using namespace std;

const int ROOK_BITS = 20;
const int BISHOP_BITS = 20;

const int ROOK_SHIFT = 64-ROOK_BITS;
const int BISHOP_SHIFT = 64-BISHOP_BITS;

void initMoveGen(int seed = 38921083);

BB getPawnAtk(Color c, Square s);
BB getKnightAtk(Square s);
BB getRookAtk(Square s, BB occupied);
BB getBishopAtk(Square s, BB occupied);
BB getQueenAtk(Square s, BB occupied);
BB getKingAtk(Square s);

struct PosInfo { //pins and checks
    BB occ;
    BB notturn_occ;
    BB turn_occ;

    int checks = 0;
    BB check_blocking_squares = ~0ULL;
    BB moveable_squares[64] = {};
    BB pinned_mask = 0ULL;
    bool isPawnCheck = false;

    PosInfo(Pos &p);
    void add_check(BB block_squares);
    void add_pin(int square, BB moveable_squares_);
    bool is_moveable(int from, int to);
};

vector<Move> getLegalMoves(Pos& p);

void addPawnMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);
void addKnightMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);
void addBishopMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);
void addRookMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);
void addQueenMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);
void addKingMoves(vector<Move> &moves, Pos &p, PosInfo &posInfo);

