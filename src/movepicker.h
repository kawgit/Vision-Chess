#include <vector>

#include "types.h"
#include "pos.h"
#include "thread.h"

class MovePicker {

    public:
    
    MovePickerStage stage = STAGE_NONE;
    size_t num_left;
    size_t num_left_in_stage;

    std::vector<Move> moves;
    std::vector<Score> scores;

    Square enemy_king_square;

    Move tt_move;
    Move from_to_counter;
    Move piece_to_counter;
    Move piece_from_counter;

    BB undefended_bbs[N_COLORS];
    BB tempo_bbs  [N_PIECES];
    BB check_bbs  [N_PIECES];
    BB unsafe_bbs [N_PIECES];

    bool critical_situation = false;

    Pos* pos;
    const History* history;

    public:
    MovePicker(Pos* pos_, const Move tt_move_, const History* history_);
    
    template<MovePickerStage STAGE>
    Score score_move(const Move move);

    template<MovePickerStage STAGE>
    void score_moves();

    Move pop();

    void next_stage();

    bool has_move();
};