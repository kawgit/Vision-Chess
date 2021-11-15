#pragma once
#include "types.h"
#include "pos.h"
#include <vector>

const int ROOK_BITS = 16;
const int BISHOP_BITS = 11;

const int ROOK_SHIFT = 64-ROOK_BITS;
const int BISHOP_SHIFT = 64-BISHOP_BITS;



void initM(int seed = 32123);


BB getPawnAtk(Square s);
BB getKnightAtk(Square s);
BB getRookAtk(Square s, BB &occupied);
BB getBishopAtk(Square s, BB &occupied);
BB getQueenAtk(Square s, BB &occupied);
BB getKingAtk(Square s);

void addLegalMoves(Pos& p, vector<Move> &moves);

PNC getPNC(Pos &p);

void addPawnMoves(vector<Move> &moves, Pos &p, PNC &pnc);
void addKnightMoves(vector<Move> &moves, Pos &p, PNC &pnc);
void addBishopMoves(vector<Move> &moves, Pos &p, PNC &pnc);
void addRookMoves(vector<Move> &moves, Pos &p, PNC &pnc);
void addQueenMoves(vector<Move> &moves, Pos &p, PNC &pnc);
void addKingMoves(vector<Move> &moves, Pos &p, PNC &pnc);