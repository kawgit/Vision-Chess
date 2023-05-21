#include "pos.h"
#include "types.h"
#include "search.h"
#include <vector>
#include <algorithm>
#include <cstdlib>

// return value is point gain for p1, so +1 = p1 won, 0 = draw, -1 = p1 lost
int simulate_game(NNUE& p1, NNUE& p2) {
    Pos p1_pos("2n4k/1ppp1pp1/8/8/8/8/1PPP1PP1/2N4K w - - 0 1"); p1_pos.nnue = &p1;
    Pos p2_pos("2n4k/1ppp1pp1/8/8/8/8/1PPP1PP1/2N4K w - - 0 1"); p2_pos.nnue = &p2;
    for (int ply = 0; ply < 50; ply++) {
        // print(p1_pos);

        Move p1_move = get_best_move(p1_pos, 1);
        p1_pos.do_move(p1_move);
        p2_pos.do_move(p1_move);
        if (p1_pos.is_over()) break;

        Move p2_move = get_best_move(p2_pos, 1);
        p1_pos.do_move(p2_move);
        p2_pos.do_move(p2_move);
        if (p1_pos.is_over()) break;
    }

    if (!p1_pos.is_over()) {
        int mat = p1_pos.ref_mat(WHITE) - p1_pos.ref_mat(BLACK);
        return mat / abs(mat);
    }

    if (p1_pos.is_draw()) return 0;
    if (p1_pos.turn == BLACK) return 1;
    if (p1_pos.turn == WHITE) return -1;
    
    assert(false);
    return 0; // should never happen
}

void train(string nnue_path, int num_i, int num_participants) {
    srand(get_current_ms());

    for (int i = 0; i < num_i; i++) {
        vector<NNUE> nnues = {};
        for (int i = 0; i < num_participants; i++) {
            nnues.emplace_back(nnue_path);
            nnues.back().add_blurry_bonus(rand() % 64, ((rand() % 2 == 0) ? KING : PAWN), rand() % 13 - 6);
        }
        vector<int> scores(nnues.size(), 0);
        for (int p1 = 0; p1 < nnues.size(); p1++) {
            for (int p2 = p1 + 1; p2 < nnues.size(); p2++) {
                int result = simulate_game(nnues[p1], nnues[p2]);
                scores[p1] += result;
                scores[p2] -= result;
            }
        }


        int index_of_highest_score = distance(scores.begin(), max_element(scores.begin(), scores.end()));
        nnues[index_of_highest_score].save(nnue_path);

        cout << "iteration: " << i << endl;
        cout << "max score: " << scores[index_of_highest_score] << endl;
        nnues[index_of_highest_score].print_maps();
    }


}


