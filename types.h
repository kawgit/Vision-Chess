#pragma once

#include <stdint.h>
#include <string>
#include <vector>

using namespace std;

typedef uint64_t BB; //bitboard
typedef uint8_t Square;
typedef uint8_t Piece;
typedef bool Color;
typedef int16_t Eval;
typedef uint8_t Depth;
const Eval INF_EVAL = 32767;

//enums
enum GENERATION_MODES : uint8_t {QUIET = 0b1, CAPTURES = 0b10, CHECKS = 0b100, LEGAL = 0b11111111};
enum CR_ {WKS, WQS, BKS, BQS};
enum Colors : Color {BLACK = 0, WHITE = 1};
enum Pieces : Piece {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_P};
enum Squares : Square {
    A1 = 0, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
};
enum MOVEFLAGS : uint16_t {QUIET_F, DOUBLE_PAWN_PUSH_F, KING_CASTLE_F, QUEEN_CASTLE_F, CAPTURE_F, EP_F, N_PROM_F=8, B_PROM_F, R_PROM_F, Q_PROM_F, N_PROM_CAP_F, B_PROM_CAP_F, R_PROM_CAP_F, Q_PROM_CAP_F, IS_CHECK_F};
enum RAY_DIRECTIONS {NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST};

Color getOppositeColor(Color c);

struct Move {
    const static int FLAG_SHIFT = 18;

    uint32_t encode = 0;
    Eval eval = 0;

    Move() {

    }
    Move(uint32_t from, uint32_t to, uint32_t fp, uint32_t tp, uint32_t flags) {
        //encode = (flags << FLAG_SHIFT) | (fp << 15) | (tp << 12) | (from << 6) | to; 
        encode = ((flags & 0b11111) << FLAG_SHIFT) | ((fp & 0b111) << 15) | ((tp & 0b111) << 12) | ((from & 0b111111) << 6) | (to & 0b111111);
    }

   uint32_t getTo() const {return encode & 0b111111;}
   uint32_t getFr() const {return (encode >> 6) & 0b111111;}

   uint32_t getTp() const {return (encode >> 12) & 0b111;}
   uint32_t getFp() const {return (encode >> 15) & 0b111;}
   uint32_t getFlags() const {return (encode >> FLAG_SHIFT) & 0b1111;}
   Piece getProm() const {return (getFlags() & 0b0011) + KNIGHT;}

   bool isCapture() const {return getFlags() & CAPTURE_F;}
   bool isPromotion() const {return getFlags() & N_PROM_F;}
   bool isDoublePawnPush() const {return getFlags() == DOUBLE_PAWN_PUSH_F;}
   bool isKC() const {return getFlags() == KING_CASTLE_F;}
   bool isQC() const {return getFlags() == QUEEN_CASTLE_F;}
   bool isEp() const {return getFlags() == EP_F;}
   bool isCheck() const {return (encode>>FLAG_SHIFT) & IS_CHECK_F;}

   string getSAN();
};

void order(vector<Move> &moveList, Move table_move, Move huerist_move);

void sort(vector<Move> &moveList);

void insertToSortedMoveList(vector<Move> &moveList, Move m);

struct CR { //castle rights
    uint8_t bits = 0;

    inline void setFull() { bits = 0b1111;}

    inline void rmCR(CR_ CR) { bits &= ~(1<<CR);}

    inline void adWK() { bits |= 0b0001;}
    inline void adWQ() { bits |= 0b0010;}
    inline void adBK() { bits |= 0b0100;}
    inline void adBQ() { bits |= 0b1000;}

    inline void rmWK() { bits &= ~0b0001;}
    inline void rmWQ() { bits &= ~0b0010;}
    inline void rmBK() { bits &= ~0b0100;}
    inline void rmBQ() { bits &= ~0b1000;}

    inline bool getWK() { return bits & 0b0001;}
    inline bool getWQ() { return bits & 0b0010;}
    inline bool getBK() { return bits & 0b0100;}
    inline bool getBQ() { return bits & 0b1000;}

    inline void operator=(CR a) { bits = a.bits;}
    inline bool operator==(CR a) { return bits == a.bits; }
    inline bool operator!=(CR a) { return bits != a.bits; }
};


struct SPiece { //specific piece
    Color c;
    Piece p;
    SPiece(Color C, Piece P) {
        c = C;
        p = P;
    }
};



//BB MASKS
extern BB square_masks[64];
extern BB file_masks[8];
extern BB rank_masks[8];

//init BB masks
void initBB();

inline BB getBB(int s) { return square_masks[s]; }

inline BB getRankBB(int r) { return rank_masks[r]; }

inline BB getFileBB(int f) { return file_masks[f]; }

struct PNC { //pins and checks
    int checks = 0;
    BB check_block_squares = ~0ULL;
    BB moveable_squares[64] = {};
    BB pinned_mask = 0ULL;
    bool isPawnCheck = false;

    inline void add_check(BB block_squares) {
        check_block_squares &= block_squares;
        checks++;
    }

    inline void add_pin(int square, BB moveable_squares_) {
		moveable_squares[square] = moveable_squares_;
		pinned_mask |= getBB(square);
	}

    inline bool is_moveable(int from, int to) {
		return ((!(pinned_mask & getBB(from))) || (moveable_squares[from] & getBB(to))) && (check_block_squares & getBB(to)); 
	}
};