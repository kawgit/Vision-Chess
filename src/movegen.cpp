#include <cassert>
#include <vector>

#include "attacks.h"
#include "bits.h"
#include "move.h"
#include "movegen.h"
#include "pos.h"
#include "types.h"

namespace movegen {

    void add_pawns_from_bb(std::vector<Move>& moves, BB to_squares, const Square offset, const MoveFlag flags) {
        while (to_squares) {
            Square to_square = bb_pop(to_squares);
            Square from_square = to_square - offset;
            moves.push_back(make_move(from_square, to_square, flags));
        }
    }

    template<GenType GENTYPE, Color COLOR>
    void add_pawn_moves(std::vector<Move>& moves, Pos& pos) {
        BB pawns = pos.pieces(COLOR, PAWN);
        BB occupied = pos.pieces();
        BB enemies = pos.pieces(!COLOR);
        BB ep_bb = bb_of(pos.ep());
        BB moveable = pos.moveable();

        if (!pawns) return;

        constexpr Direction forward         = COLOR == WHITE ? NORTH  : SOUTH;
        
        constexpr Rank      far_rank        = COLOR == WHITE ? RANK_8 : RANK_1;
        constexpr BB        far_rank_bb     = bb_of_rank(far_rank);
        constexpr Rank      mid_rank        = COLOR == WHITE ? RANK_4 : RANK_5;
        constexpr BB        mid_rank_bb     = bb_of_rank(mid_rank);

        constexpr Square    forward_offset  = COLOR == WHITE ? 8 : -8;
        constexpr Square    east_offset     = 1;

        BB p1 = bb_shift<forward>(pawns) & ~occupied;
        BB p2 = bb_shift<forward>(p1)    & ~occupied & mid_rank_bb;
        
        BB d1 = bb_shift<EAST>(bb_shift<forward>(pawns));
        BB d2 = bb_shift<WEST>(bb_shift<forward>(pawns));
        
        BB ep1 = d1 & ep_bb;
        BB ep2 = d2 & ep_bb;

        p1 &= moveable;
        p2 &= moveable;
        
        d1 &= moveable;
        d2 &= moveable;
        
        BB c1 = d1 & enemies;
        BB c2 = d2 & enemies;
        
        BB prom_p1 = p1 & far_rank_bb;
        BB prom_c1 = c1 & far_rank_bb;
        BB prom_c2 = c2 & far_rank_bb;

        p1 &= ~far_rank_bb;
        c1 &= ~far_rank_bb;
        c2 &= ~far_rank_bb;

        // Add moves roughly in order of promise. This has no purpose for movepicker, but benefits sorting algorithms. On the whole not necessary. 

        if constexpr (GENTYPE & FLAG_TACTICALS) {
            add_pawns_from_bb(moves, prom_c1, forward_offset + east_offset, Q_PROM_CAPTURE);
            add_pawns_from_bb(moves, prom_c2, forward_offset - east_offset, Q_PROM_CAPTURE);
            add_pawns_from_bb(moves, prom_p1, forward_offset,               Q_PROM);
            add_pawns_from_bb(moves, prom_c1, forward_offset + east_offset, N_PROM_CAPTURE);
            add_pawns_from_bb(moves, prom_c2, forward_offset - east_offset, N_PROM_CAPTURE);
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
            add_pawns_from_bb(moves, prom_c1, forward_offset + east_offset, B_PROM_CAPTURE);
            add_pawns_from_bb(moves, prom_c2, forward_offset - east_offset, B_PROM_CAPTURE);
            add_pawns_from_bb(moves, prom_p1, forward_offset,               B_PROM);
            add_pawns_from_bb(moves, prom_c1, forward_offset + east_offset, R_PROM_CAPTURE);
            add_pawns_from_bb(moves, prom_c2, forward_offset - east_offset, R_PROM_CAPTURE);
            add_pawns_from_bb(moves, prom_p1, forward_offset,               R_PROM);
        }
    }

    template<GenType GENTYPE, Color COLOR, Piece PIECE>
    void add_piece_moves(std::vector<Move>& moves, Pos& pos) {
        
        BB pieces = pos.pieces(COLOR, PIECE);
        BB occupied = pos.pieces();
        BB moveable = pos.moveable();

        while (pieces) {
            
            Square from_square = bb_pop(pieces);
            
            BB to_squares = attacks::lookup(PIECE, from_square, occupied) & ~pos.pieces(COLOR) & moveable;
            
            if constexpr (GENTYPE & FLAG_TACTICALS) {

                BB captures = to_squares & occupied;

                while (captures) {
                    Square to_square = bb_pop(captures);
                    moves.push_back(make_move(from_square, to_square, CAPTURE));
                }

            }


            if constexpr (GENTYPE & FLAG_QUIETS) {

                BB quiets = to_squares & ~occupied;

                while (quiets) {
                    Square to_square = bb_pop(quiets);
                    moves.push_back(make_move(from_square, to_square, QUIET));
                }

            }
        }
    }

    template<GenType GENTYPE, Color COLOR>
    void add_king_moves(std::vector<Move>& moves, Pos& pos) {

        Square from_square = bb_peek(pos.pieces(COLOR, KING));
        BB to_squares = attacks::king(from_square) & ~pos.pieces(COLOR);
        
        if constexpr (GENTYPE & FLAG_LEGAL) {
            to_squares &= ~pos.attacked_by(pos.notturn());
        }

        if constexpr (GENTYPE & FLAG_TACTICALS) {

            BB captures = to_squares & pos.pieces(!COLOR);

            while (captures) {
                Square to_square = bb_pop(captures);
                moves.push_back(make_move(from_square, to_square, CAPTURE));
            }

        }

        if constexpr (GENTYPE & FLAG_QUIETS) {

            BB quiets = to_squares & ~pos.pieces(!COLOR);

            while (quiets) {
                Square to_square = bb_pop(quiets);
                moves.push_back(make_move(from_square, to_square, QUIET));
            }

            constexpr Square AX = COLOR == WHITE ? A1 : A8;
            constexpr Square BX = COLOR == WHITE ? B1 : B8;
            constexpr Square CX = COLOR == WHITE ? C1 : C8;
            constexpr Square DX = COLOR == WHITE ? D1 : D8;
            constexpr Square EX = COLOR == WHITE ? E1 : E8;
            constexpr Square FX = COLOR == WHITE ? F1 : F8;
            constexpr Square GX = COLOR == WHITE ? G1 : G8;
            constexpr Square HX = COLOR == WHITE ? H1 : H8;
            
            if (bb_has(pos.cr(), HX) && !((pos.attacked_by(pos.notturn()) & bb_segment(DX, HX)) || (pos.pieces() & bb_segment(EX, HX))))
                moves.push_back(make_move(EX, GX, KING_CASTLE));

            if (bb_has(pos.cr(), AX) && !((pos.attacked_by(pos.notturn()) & bb_segment(FX, BX)) || (pos.pieces() & bb_segment(EX, AX))))
                moves.push_back(make_move(EX, CX, QUEEN_CASTLE));
        }

    }

    template<GenType GENTYPE, Color COLOR>
    void add_moves(std::vector<Move>& moves, Pos& pos) {

        if constexpr (GENTYPE & FLAG_LEGAL) {
            pos.update_legal_info();
        }

        if (!(GENTYPE & FLAG_LEGAL) || !bb_has_multiple(pos.checkers())) {

            add_pawn_moves <GENTYPE, COLOR>         (moves, pos);
            add_piece_moves<GENTYPE, COLOR, KNIGHT> (moves, pos);
            add_piece_moves<GENTYPE, COLOR, BISHOP> (moves, pos);
            add_piece_moves<GENTYPE, COLOR, ROOK>   (moves, pos);
            add_piece_moves<GENTYPE, COLOR, QUEEN>  (moves, pos);

            if constexpr (GENTYPE & FLAG_LEGAL) {

                BB pinned = pos.pinned();
                
                size_t i = 0;
                while (i != moves.size()) {
                    const bool is_pinned = bb_has(pinned, move::from_square(moves[i]));
                    const bool is_ep     = move::is_ep(moves[i]);

                    if ((is_pinned || is_ep) && !pos.is_legal(moves[i])) {
                        moves[i] = moves.back();
                        moves.pop_back();
                    }                
                    else
                        i++;
                }

            }
        }
        
        add_king_moves <GENTYPE, COLOR> (moves, pos);

    }

    template std::vector<Move> generate<PSEUDO>(Pos& pos);
    template std::vector<Move> generate<LEGAL >(Pos& pos);
    template std::vector<Move> generate<LOUDS >(Pos& pos);

    template<GenType GENTYPE>
    std::vector<Move> generate(Pos& pos) {
        std::vector<Move> moves;
        moves.reserve(300);

        if (pos.turn() == WHITE)
            add_moves<GENTYPE, WHITE>(moves, pos);
        else
            add_moves<GENTYPE, BLACK>(moves, pos);
        
        return moves;
    }

}