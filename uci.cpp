#include "search.h"
#include "bits.h"
#include "types.h"
#include "pos.h"
#include "tt.h"
#include "movegen.h"
#include "uci.h"
#include "eval.h"
#include <sstream>
#include <cassert>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>
#include <cstdlib>

#ifndef _WIN32
#include <unistd.h>
#define sleep(ms) usleep(ms * 1000)
#endif
#ifdef _WIN32
#include <Windows.h>
#define sleep(ms) Sleep(ms)
#endif


using namespace std;


vector<UCI_Option> uci_options = {
    UCI_Option("Ponder", "check", "true", "option name Ponder type check default true"),
    UCI_Option("Hash", "spin", "512", "option name Hash type spin default 512 min 16 max 1024"),
    UCI_Option("OwnBook", "check", "false", "option name OwnBook type check default false"),
    UCI_Option("Threads", "spin", "1", "option name Threads type spin default 1 min 1 max 8"),
};

UCI_Option* get_uci_option(string name) {
    for (UCI_Option& uci_option : uci_options) {
        if (uci_option.name == name) return &uci_option;
    }
    return nullptr;
}

void uci() {
    cout.setf(ios::unitbuf);

    SearchInfo uci_si;
    
    string token;
    while (true) {
        cin >> token;

        if (token == "uci") {
            cout << "id name VisionChess" << endl;
            cout << "id author Kenneth Wilber" << endl;

            for (UCI_Option option : uci_options) {
                cout << option.init << endl;
            }

            cout << "uciok" << endl;
        }
        else if (token == "isready") {
            cout << "readyok" << endl;
        }
        else if (token == "setoption") {
            cin >> token;
            assert(token == "name");
            cin >> token;
            bool found = false;
            for (UCI_Option& option : uci_options) {
                if (option.name == token) {
                    found = true;
                    cin >> token;
                    assert(token == "value");
                    cin >> token;
                    option.value = token;
                }
            }

            if (!found) {
                cout << "no option named " << token << endl;
            }
        }
        else if (token == "ucinewgame") {
            uci_si.stop();
            uci_si.clear();
        }
        else if (token == "position") {
            string line;
            getline(cin, line);
            istringstream iss(line);
            
            iss >> token;
            if (token == "fen") {
                string part;
                string fen = "";
                for (int i = 0; i < 6; i++) {
                    iss >> part;
                    fen += part + " ";
                }
                uci_si.root_pos = Pos(fen);
            }
            else if (token == "startpos") {
                uci_si.root_pos = Pos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            }
            else {
                assert(false);
            }
            
            if (iss >> token) {
                assert(token == "moves");

                while (iss >> token) {
                    uci_si.root_pos.do_move(token);
                }
            }

            uci_si.tt.gen++;
        }
        else if (token == "go") {
            string line;
            getline(cin, line);
            istringstream iss(line);

            Timestamp wtime = -1;
            Timestamp btime = -1;
            Timestamp winc =  0;
            Timestamp binc =  0;

            uci_si.ponder = false;
            uci_si.num_threads = stoi(get_uci_option("Threads")->value);
            uci_si.max_time = -1;
            uci_si.max_depth = DEPTHMAX;
            uci_si.last_depth_searched = 0;

            while (iss >> token) {
                if (token == "ponder") {
                    uci_si.ponder = true;
                }
                else if (token == "wtime") {
                    iss >> token;
                    wtime = stoi(token);
                }
                else if (token == "btime") {
                    iss >> token;
                    btime = stoi(token);
                }
                else if (token == "winc") {
                    iss >> token;
                    winc = stoi(token);
                }
                else if (token == "binc") {
                    iss >> token;
                    binc = stoi(token);
                }
                else if (token == "depth") {
                    iss >> token;
                    uci_si.max_depth = stoi(token);
                }
                else if (token == "infinite") {
                    uci_si.max_depth = DEPTHMAX;
                    uci_si.max_time = -1;
                }
            }

            uci_si.max_time = (uci_si.root_pos.turn == WHITE ? min((wtime / 30 + winc), wtime / 2) : min((btime / 30 + binc), btime / 2));

            bool verbose = true;
            thread(&SearchInfo::launch, &uci_si, ref(verbose)).detach();
            sleep(10);
        }
        else if (token == "stop") {
            uci_si.stop();
        }
        else if (token == "ponderhit") {
            uci_si.start_time = get_current_ms();
            uci_si.ponder = false;
            uci_si.print();
        }
        else if (token == "d") {
            print(uci_si.root_pos, true);
        }
        else if (token == "r") {
            Eval eval = eval_pos(uci_si.root_pos, -INF, INF, true);
            cout << "Eval: " << eval_to_string(eval) << endl;
        }
        else if (token == "quit") {
            uci_si.stop();
            break;
        }
    }

}