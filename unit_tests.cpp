#include "unit_tests.h"
#include "movegen.h"
#include "hash.h"
#include "timer.h"
#include "search.h"
#include "pos.h"
#include "bits.h"
#include "types.h"
#include <iomanip>
#include <cassert>
#include <exception>
#include <thread>

using namespace std;

string padTo(std::string str, const size_t num, const char paddingChar = ' ')
{
    if(num > str.size())
        str.insert(str.size()-1, num - str.size(), paddingChar);
    
    return str;
}

int passed = 0;
int failed = 0;
int total = 0;

void require(bool condition, vector<vector<string>> metadata = {}) {
    if (condition) {
#ifndef CSV_OUTPUT
        cout << "\033[1;33m";
        cout << "pass ";
        cout << "\033[0m";
#endif
        passed++;
    }
    else {
#ifndef CSV_OUTPUT
        cout << "\033[1;31m";
        cout << "fail ";
        cout << "\033[0m";
#endif
        failed++;
    }
    total++;
#ifndef CSV_OUTPUT
    for (vector<string> record : metadata) {
        assert(record.size());
        switch (record.size()) {
            case 1:
                cout << setw(10) << record[0] << " ";
                break;
            case 2:
                cout << setw(10) << record[0] << ": " << setw(10) << record[1] << " ";
                break;
            defualt:
                for (string str : record) {
                    cout << setw(10) << str << " ";
                }
                break;
        }
    }
    cout << endl;
#endif
#ifdef CSV_OUTPUT
    int score = 0;
    if (condition) {
        score += stoi(metadata[0][1]);
    }
    else {
        score += 15000;
    }
    cout << score << ",";
#endif
}

vector<Test> tests = {
    Test("perft", [] {
        vector<string> fens = {
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ",
		};
        vector<BB>   counts = {
            4865609,
            193690690,
            674624,
            15833292,
            15833292,
            89941194,
            164075551,
        };

        for (int i = 0; i < fens.size(); i++) {
            Pos p(fens[i]);
            Timestamp start = get_current_ms();
            BB count = perft(p, 5, false);
            require(count == counts[i], {
                {"time", to_string(get_time_diff(start)) + "ms"},
                {"nps", to_string(count*1000/get_time_diff(start))},
                {"fen", fens[i]},
            });
        }
    }),

    Test("puzzles", [] {
        vector<vector<string>> tests = {
            {"2br4/2R3bk/5p1p/5Pp1/2BpP1P1/1P5P/P7/6K1 w - - 3 34", "e4e5"},
            {"b1r3k1/4ppb1/p2p2p1/7p/2rBP3/P4P2/4N1PP/1RR3K1 w - - 0 30", "c1c4"},
            {"1r6/1p3p1k/2pp2Rp/4p3/r1B1Pq2/2bP1P1P/4KP2/3R2Q1 w - - 1 28", "g6g7"},
            {"7R/7p/5P1k/4PKpP/8/8/8/8 w - g6 0 2", "h5g6"},
            {"b7/PP6/8/8/7K/6B1/6N1/4R1bk w - - 0 1", "b7a8n"},
            {"8/8/8/8/7k/8/PPPPPPPP/RNBQKBNR w KQ - 0 1", "d2d4"},
            {"5k2/8/6K1/3P2R1/8/8/8/8 w - - 0 1", "g5e5"},
            {"1q6/2r3bk/4Q1pp/2pBP3/p1P5/6P1/1P1p1RK1/8 w - - 0 49", "d5e4"},
            {"3r3r/1k6/2p4q/p1Pp1p2/3Rp1p1/1P2P1n1/PB2Q2P/5BK1 w - - 0 27", "e2a6"},
            {"r1b1kb1r/pp1p1ppp/5n2/4p3/2Pn4/P1N1B3/1q2PPPP/R2QKBNR w KQkq - 0 9", "e3d4"},
            {"5rk1/3R4/4p1pR/8/3b4/1Pp2P1P/1n2K1P1/8 b - - 1 38", "f8c8"},
            {"4r2k/1p5p/p2P4/5p2/8/5PK1/PP6/3R4 b - - 2 34", "h8g7"},
            {"4r2k/1p5p/p2P4/3Q1pp1/7P/4qP2/PP1n2P1/3R3K b - h3 0 25", "e3e1"},
            {"1r1q1rk1/2p1np2/bp2p1pp/p1PpP3/7P/1PR1P1P1/PB1N1P2/3Q2RK b - - 0 23", "d5d4"}
        };

        for (vector<string> test : tests) {
            Pos pos(test[0]);
            SearchInfo si;
            Timestamp min_time = 1000;
            Timestamp max_time = 10000;
            ThreadInfo ti(pos, "puzzle_thread");
            ThreadInfo* ti_ptr = &ti;

            thread timer_thread(&timer, ref(ti_ptr->searching), ref(max_time));

            bool found_solution = false;
            Timestamp start = get_current_ms();

            int depth;
            for (depth = 1; depth < 127 && ti.searching; depth++) {
                search(pos, depth, -INF, INF, ti, si);

                bool found;
                string move_str = to_san(si.tt.probe(pos.ref_hashkey(), found)->get_move());
                bool found_valid_move = false;
                for (int i = 1; i < test.size(); i++) {
                    if (test[i] == move_str) {
                        found_valid_move = true;
                    }
                }

                if (found_valid_move && get_time_diff(start) > min_time) {
                    break;
                }
            }

            ti.searching = false;
            timer_thread.join();

            bool found;
            require(to_san(si.tt.probe(pos.ref_hashkey(), found)->get_move()) == test[1], {
                {"time", to_string(get_time_diff(start)) + "ms"},
                {"nps", to_string(ti.nodes*1000/(get_time_diff(start)+1))},
                {"nodes", to_string(ti.nodes)},
                {"depth", to_string(depth)},
                {"pv", padTo(to_string(si.tt.getPV(pos)).substr(0, 40), 40)},
                {"fen", test[0]},
            });
        }
    })
    
};


int main() {
    init_hash(320193);
    init_movegen();


#ifndef CSV_OUTPUT
    for (Test test : tests) {
        cout << "TEST SECTION: " << test.name << endl;
        Timestamp start = get_current_ms();
        test.func();
    }

    cout << "passed: " << passed << endl;
    cout << "failed: " << failed << endl;
    cout << "total: " << total << endl;

    if (!failed) {
        cout << "\033[1;33m";
        cout << "     ====================    " << endl;
        cout << "  ~~ | all tests passed | ~~ " << endl;
        cout << "  ~~ | all tests passed | ~~ " << endl;
        cout << "  ~~ | all tests passed | ~~ " << endl;
        cout << "     ====================    " << endl;
        cout << "\033[0m";
    }
    else {
        cout << "\033[1;31m";
        cout << "     ====================    " << endl;
        cout << "     |     X     X      |    " << endl;
        cout << "     |     _-----_      |    " << endl;
        cout << "     |    -       -     |    " << endl;
        cout << "     ====================    " << endl;
        cout << "\033[0m";
    }
#endif
#ifdef CSV_OUTPUT
    for (Test test : tests) {
        test.func();
    }
#endif

    return 0;
}

