#include <vector>
#include <cassert>

#include "attacks.h"
#include "bits.h"
#include "move.h"
#include "movegen.h"
#include "pos.h"
#include "types.h"

using namespace std;

namespace movegen {

    void add_pawns_from_bb(vector<Move>& moves, BB to_squares, const Square offset, const MoveFlag flags) {
        while (to_squares) {
            Square to_square = poplsb(to_squares);
            Square from_square = to_square - offset;
            moves.push_back(make_move(from_square, to_square, flags));
        }
    }

    template<GenType GENTYPE, Color COLOR>
    void add_pawn_moves(vector<Move>& moves, Pos& pos) {
        BB pawns = pos.pieces(COLOR, PAWN);
        BB occupied = pos.pieces();
        BB enemies = pos.pieces(!COLOR);
        BB ep_bb = bb_of(pos.ep());

        if (!pawns) return;

        constexpr Direction forward         = COLOR == WHITE ? NORTH  : SOUTH;
        
        constexpr Rank      far_rank        = COLOR == WHITE ? RANK_8 : RANK_1;
        constexpr BB        far_rank_bb     = bb_of(far_rank);

        constexpr Square    forward_offset  = COLOR == WHITE ? 8 : -8;
        constexpr Square    east_offset     = 1;

        BB p1 = shift<forward>(pawns) & ~occupied;
        BB p2 = shift<forward>(p1)    & ~occupied;
        
        BB d1 = shift<EAST>(shift<forward>(pawns));
        BB d2 = shift<WEST>(shift<forward>(pawns));
        
        BB c1 = d1 & enemies;
        BB c2 = d2 & enemies;
        
        BB ep1 = d1 & ep_bb;
        BB ep2 = d2 & ep_bb;
        
        BB prom_p1 = p1 & far_rank_bb;
        BB prom_c1 = c1 & far_rank_bb;
        BB prom_c2 = c2 & far_rank_bb;

        p1 &= ~far_rank_bb;
        c1 &= ~far_rank_bb;
        c2 &= ~far_rank_bb;

        // Add moves roughly in order of promise. This has no purpose for movepicker, but benefits sorting algorithms. On the whole not necessary. 

        if constexpr (GENTYPE & FLAG_TACTICALS) {
            add_pawns_from_bb(moves, prom_c1, forward_offset + east_offset, Q_PROM);
            add_pawns_from_bb(moves, prom_c2, forward_offset - east_offset, Q_PROM);
            add_pawns_from_bb(moves, prom_p1, forward_offset,               Q_PROM);
            add_pawns_from_bb(moves, prom_c1, forward_offset + east_offset, N_PROM);
            add_pawns_from_bb(moves, prom_c2, forward_offset - east_offset, N_PROM);
            add_pawns_from_bb(moves, prom_p1, forward_offset,               N_PROM);

            add_pawns_from_bb(moves, ep1, forward_offset + east_offset, EP);
            add_pawns_from_bb(moves, ep2, forward_offset - east_offset, EP);

            add_pawns_from_bb(moves, c1,  forward_offset + east_offset, CAPTURE);
            add_pawns_from_bb(moves, c2,  forward_offset - east_offset, CAPTURE);
        }

        if constexpr (GENTYPE & FLAG_QUIETS) {
            add_pawns_from_bb(moves, p1,  forward_offset, QUIET);
            add_pawns_from_bb(moves, p2,  forward_offset + forward_offset, DOUBLE_PAWN_PUSH);
        }

        if constexpr (GENTYPE & FLAG_TACTICALS) {
            add_pawns_from_bb(moves, prom_c1, forward_offset + east_offset, B_PROM);
            add_pawns_from_bb(moves, prom_c2, forward_offset - east_offset, B_PROM);
            add_pawns_from_bb(moves, prom_p1, forward_offset,               B_PROM);
            add_pawns_from_bb(moves, prom_c1, forward_offset + east_offset, R_PROM);
            add_pawns_from_bb(moves, prom_c2, forward_offset - east_offset, R_PROM);
            add_pawns_from_bb(moves, prom_p1, forward_offset,               R_PROM);
        }
    }

    template<GenType GENTYPE, Color COLOR, Piece PIECE>
    void add_piece_moves(vector<Move>& moves, Pos& pos) {
        
        BB pieces = pos.pieces(COLOR, PIECE);
        BB occupied = pos.pieces();

        while (pieces) {
            
            Square from_square = poplsb(pieces);
            
            BB to_squares = attacks::lookup(PIECE, from_square, occupied) & ~pos.pieces(COLOR);
            
            if constexpr (GENTYPE & FLAG_TACTICALS) {

                BB captures = to_squares & pos.pieces(!COLOR);

                while (captures) {
                    Square to_square = poplsb(captures);
                    moves.push_back(make_move(from_square, to_square, CAPTURE));
                }

            }


            if constexpr (GENTYPE & FLAG_QUIETS) {

                BB quiets = to_squares & ~pos.pieces(!COLOR);

                while (quiets) {
                    Square to_square = poplsb(quiets);
                    moves.push_back(make_move(from_square, to_square, QUIET));
                }

            }
        }
    }

    template<GenType GENTYPE, Color COLOR>
    void add_moves(vector<Move>& moves, Pos& pos) {

        add_pawn_moves <GENTYPE, COLOR>         (moves, pos);
        add_piece_moves<GENTYPE, COLOR, KNIGHT> (moves, pos);
        add_piece_moves<GENTYPE, COLOR, BISHOP> (moves, pos);
        add_piece_moves<GENTYPE, COLOR, ROOK>   (moves, pos);
        add_piece_moves<GENTYPE, COLOR, QUEEN>  (moves, pos);
        add_piece_moves<GENTYPE, COLOR, KING>   (moves, pos);

    }

    template vector<Move> generate<PSEUDO>(Pos& pos);

    template<GenType GENTYPE>
    vector<Move> generate(Pos& pos) {
        vector<Move> moves;
        moves.reserve(300);

        if (pos.turn() == WHITE)
            add_moves<GENTYPE, WHITE>(moves, pos);
        else
            add_moves<GENTYPE, BLACK>(moves, pos);
        
        return moves;
    }

}