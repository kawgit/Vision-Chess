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
#include "stdlib.h"

#ifdef _WIN32
#include <Windows.h>
#define sleep(t) Sleep(t * 1000)
#else
#include <unistd.h>
#endif
#include <iostream>
#include <cstdlib>

using namespace std;


Pos uci_root_pos;
SearchInfo uci_root_si;
vector<ThreadInfo> uci_root_tis;

bool one_thread_active() {
    for (int i = 0; i < uci_root_tis.size(); i++) {
        if (uci_root_tis[i].searching) return true;
    }

    return false;
}

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

mutex worker_mutex;
mutex print_mutex;

void print_uci_info(Pos& pos, SearchInfo& si, vector<ThreadInfo>& tis) {
    bool found = false;
    TTEntry* entry = si.tt.probe(pos.hashkey, found);
    
    if (!found) return;

    BB nodes = 0;

    for (ThreadInfo ti : tis) {
        nodes += ti.nodes;
    }

    Timestamp time_elapsed = get_time_diff(si.start);

    string msg = "";

    msg += "info";
    msg += " depth " + to_string(entry->get_depth());
    msg += " score " + eval_to_string(entry->get_eval());
    msg += " nodes " + to_string(nodes);
    msg += " time " + to_string(time_elapsed);
    msg += " nps " + to_string(time_elapsed != 0 ? (nodes * 1000 / time_elapsed) : 0);
    msg += " hashfull " + to_string(si.tt.hashfull());
    msg += " pv " + to_string(si.tt.getPV(pos)) + "\n";

    print_mutex.lock();

    cout << msg;
    
    print_mutex.unlock();
}

void worker(ThreadInfo& ti) {

    while (ti.searching && uci_root_si.is_active) {

        worker_mutex.lock();

        Depth depth = ++uci_root_si.last_depth_searched;

        Pos pos = uci_root_pos;

        worker_mutex.unlock();

        if (depth > uci_root_si.max_depth || depth < 0) break;
        
        search(pos, depth, -INF, INF, ti, uci_root_si);

        print_uci_info(uci_root_pos, uci_root_si, uci_root_tis);
    }
    
    ti.searching = false;

}

void uci_stop() {
    for (ThreadInfo& ti : uci_root_tis) {
        ti.searching = false;
    }
    uci_root_si.is_active = false;
    uci_root_si.last_depth_searched = 0;
}

void uci_clear() {
    uci_root_si.tt.clear();
}

void uci_search() {
    uci_stop();
    uci_root_tis.clear();
    
    uci_root_si.is_active = true;
    uci_root_si.start = get_current_ms();
    
    vector<thread> threads;
    int num_threads = stoi(get_uci_option("Threads")->value);
    for (int i = 0; i < num_threads; i++) {
        uci_root_tis.emplace_back(uci_root_pos, to_string(i));
    }

    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(&worker, ref(uci_root_tis[i]));
    }

    while (true) {
        if (!uci_root_si.is_active
            || (!uci_root_si.ponder 
                && get_time_diff(uci_root_si.start) > uci_root_si.max_time)) {
            break;
        }

        //print_uci_info(uci_root_pos, uci_root_si, uci_root_tis);
        sleep(.01);
    }
    
    //cout << "stopped" << endl;
    uci_stop();

    vector<Move> pv = uci_root_si.tt.getPV(uci_root_pos);

    vector<Move> moves = getLegalMoves(uci_root_pos);

    print_mutex.lock();
    cout << "bestmove " + (pv.size() > 0 ? getSAN(pv[0]) : (moves.size() ? getSAN(moves[0]) : "(none)")) + (pv.size() > 1 ? " ponder " + getSAN(pv[1]) : "") << endl;
    print_mutex.unlock();
    
    for (thread& thr : threads) {
        thr.join();
    }
    
}

void uci() {

    cout.setf(ios::unitbuf);
    
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
            uci_stop();
            uci_clear();
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
                uci_root_pos = Pos(fen);
            }
            else if (token == "startpos") {
                uci_root_pos = Pos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            }
            else {
                assert(false);
            }
            
            if (iss >> token) {
                assert(token == "moves");

                while (iss >> token) {
                    uci_root_pos.do_move(token);
                }
            }

            uci_root_si.tt.gen++;
        }
        else if (token == "go") {
            string line;
            getline(cin, line);
            istringstream iss(line);

            Depth depth = DEPTHMAX;
            Timestamp wtime = 3000000;
            Timestamp btime = 3000000;
            Timestamp winc =  0;
            Timestamp binc =  0;
            uci_root_si.ponder = false;

            while (iss >> token) {
                if (token == "ponder") {
                    uci_root_si.ponder = true;
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
                    depth = stoi(token);
                }
                else if (token == "infinite") {
                    depth = DEPTHMAX;
                    uci_root_si.max_time = 922337203684775807;
                }
            }

            uci_root_si.max_time = (uci_root_pos.turn == WHITE ? (wtime / 30 + winc) : (btime / 30 + binc));
            uci_root_si.max_depth = depth;

            thread(&uci_search).detach();
        }
        else if (token == "stop") {
            uci_stop();
        }
        else if (token == "ponderhit") {
            uci_root_si.start = get_current_ms();
            uci_root_si.ponder = false;
            print_uci_info(uci_root_pos, uci_root_si, uci_root_tis);
        }
        else if (token == "d") {
            print(uci_root_pos, true);
        }
        else if (token == "r") {
            Eval eval = eval_pos(uci_root_pos, -INF, INF, true);
            cout << "Eval: " << eval_to_string(eval) << endl;
        }
        else if (token == "quit") {
            uci_stop();
            break;
        }
    }

}