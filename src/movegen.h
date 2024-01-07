#pragma once

#include "pos.h"
#include "types.h"
#include "bits.h"
#include "move.h"
#include <vector>
#include <cassert>

using namespace std;

vector<Move> get_legal_moves(Pos& pos);

void add_pawn_moves(vector<Move>& moves, Pos& pos);
void add_knight_moves(vector<Move>& moves, Pos& pos);
void add_bishop_moves(vector<Move>& moves, Pos& pos);
void add_rook_moves(vector<Move>& moves, Pos& pos);
void add_queen_moves(vector<Move>& moves, Pos& pos);
void add_king_moves(vector<Move>& moves, Pos& pos);
