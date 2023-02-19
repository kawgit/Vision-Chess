#include <emscripten.h>
#include <mutex>
#include <istream>
#include <sstream>
#include "web.h"
#include "search.h"
#include "movegen.h"
#include "hash.h"
#include "pos.h"
#include "bits.h"
#include "search.h"
#include "timer.h"
#include "search.h"
#include "types.h"
#include "movegen.h"
#include "eval.h"
#include "order.h"
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

SearchInfo si;

void launch() {
    si.stop();
	si.tis.clear();

    si.is_active = true;
    si.start_time = get_current_ms();
	si.last_depth_searched = 0;
    
    for (int i = 0; i < si.num_threads; i++) {
        si.tis.emplace_back(si.root_pos, "threadname_" + to_string(i));
    }

    for (int i = 0; i < si.num_threads; i++) {
        EM_ASM_({
            let worker = new Worker('worker.js');
            worker.postMessage($0);
        }, i);
    }

    EM_ASM_({
        let timer = new Worker('timer.js');
        timer.postMessage($0);
    }, si.max_time);
}

void wasm_println(string line) {
    const char* c_str_line = line.c_str();
    EM_ASM_({
        console.log($0);
    }, c_str_line);
}

void wasm_print_uci_info() {
    bool found = false;
    TTEntry* entry = si.tt.probe(si.root_pos.ref_hashkey(), found);
    
    if (!found) return;

    BB nodes = si.get_nodes();
    Timestamp time_elapsed = get_time_diff(si.start_time);

    string msg = "";
    msg += "info";
    msg += " depth " + to_string(entry->get_depth());
    msg += " score " + eval_to_string(entry->get_eval());
    msg += " nodes " + to_string(nodes);
    msg += " time " + to_string(time_elapsed);
    msg += " nps " + to_string(nodes * 1000 / (time_elapsed + 1));
    msg += " hashfull " + to_string(si.tt.hashfull());
    msg += " pv " + to_string(si.tt.getPV(si.root_pos));

    wasm_println(msg);
}

int main() {
    init_hash(4643);
	init_movegen();
}

/*
void process_uci_line(const char* c_str_line) {

    string line(c_str_line);
    string token;
    istringstream iss(line);
    iss >> token;
    
    if (token == "uci") {
        wasm_println("id name VisionChess");
        wasm_println("id author Kenneth Wilber");

        for (UCI_Option option : uci_options) {
            wasm_println(option.init);
        }

        wasm_println("uciok");
    }
    else if (token == "isready") {
        wasm_println("readyok");
    }
    else if (token == "setoption") {
        iss >> token;
        assert(token == "name");
        iss >> token;
        bool found = false;
        for (UCI_Option& option : uci_options) {
            if (option.name == token) {
                found = true;
                iss >> token;
                assert(token == "value");
                iss >> token;
                option.value = token;
            }
        }

        if (!found) {
            wasm_println("no option named " + token);
        }
    }
    else if (token == "ucinewgame") {
        si.stop();
        si.clear();
    }
    else if (token == "position") {
        iss >> token;
        if (token == "fen") {
            string part;
            string fen = "";
            for (int i = 0; i < 6; i++) {
                assert(iss >> part);
                fen += part + " ";
            }
            si.root_pos = Pos(fen);
        }
        else if (token == "startpos") {
            si.root_pos = Pos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        }
        else {
            wasm_println("ERROR: position should be followed by 'fen' or 'startpos'");
            assert(false);
        }
        
        if (iss >> token) {
            assert(token == "moves");

            while (iss >> token) {
                si.root_pos.do_move(token);
            }
        }

        si.tt.gen++;
    }
    else if (token == "go") {
        Timestamp wtime = -1;
        Timestamp btime = -1;
        Timestamp winc =  0;
        Timestamp binc =  0;

        si.ponder = false;
        si.num_threads = stoi(get_uci_option("Threads")->value);
        si.max_time = -1;
        si.max_depth = DEPTHMAX;
        si.last_depth_searched = 0;
        bool time_is_perm = false;

        while (iss >> token) {
            if (token == "ponder") {
                si.ponder = true;
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
                si.max_depth = stoi(token);
            }
            else if (token == "infinite") {
                si.max_depth = DEPTHMAX;
                si.max_time = -1;
                time_is_perm = true;
            }
            else if (token == "perft") {
                assert(iss >> token);
                int depth = stoi(token);
                Pos pos_copy = si.root_pos;
                perft(pos_copy, depth, true);
            }
            else if (token == "movetime") {
                iss >> token;
                si.max_time = stoi(token);
                time_is_perm = true;
            }
        }

        if (!time_is_perm) si.max_time = (si.root_pos.turn == WHITE ? min((wtime / 30 + winc), wtime / 2) : min((btime / 30 + binc), btime / 2));
        launch();
        sleep(10);
    }
    else if (token == "stop") {
        web_stop();
    }
    else if (token == "ponderhit") {
        si.start_time = get_current_ms();
        si.ponder = false;
    }
    else if (token == "quit") {
        si.stop();
    }
}


void web_worker(int ti_index) {
    Pos root_copy = si.root_pos;
    ThreadInfo& ti = si.tis[ti_index];

    while (ti.searching) {
        si.depth_increment_mutex.lock();
        Depth depth = ++si.last_depth_searched;
        si.depth_increment_mutex.unlock();

        if (depth > si.max_depth || depth < 0) break;
        
        search(root_copy, depth, -INF, INF, ti, si);

		if (ti.searching) {
        	// cout << "thread " << ti.id << ":";
        	wasm_print_uci_info();
		}
    }
}

void web_stop() {
    if (si.is_active) {
        vector<Move> pv = si.tt.getPV(si.root_pos);
        vector<Move> moves = get_legal_moves(si.root_pos);
        wasm_println("bestmove " + (pv.size() > 0 ? to_san(pv[0]) : (moves.size() ? to_san(moves[0]) : "(none)")) + (pv.size() > 1 ? " ponder " + to_san(pv[1]) : ""));
        si.stop();
    }
}
*/


// const char* probe_move() {
//     bool found;
//     TTEntry* entry = si.tt.probe(si.root_pos.ref_hashkey(), found);
//     string result = found ? to_san(entry->get_move()) : "none";
//     const char* c_str_result = result.c_str();
//     return c_str_result;
// }

// int probe_eval() {
//     bool found;
//     TTEntry* entry = si.tt.probe(si.root_pos.ref_hashkey(), found);
//     return entry->get_eval();
// }

// int probe_depth() {
//     bool found;
//     TTEntry* entry = si.tt.probe(si.root_pos.ref_hashkey(), found);
//     return found ? entry->get_depth() : 0;
// }

// const char* probe_pv() {
//     string result = to_string(si.tt.getPV(si.root_pos));
//     const char* c_str_result = result.c_str();
//     return c_str_result;
// }
