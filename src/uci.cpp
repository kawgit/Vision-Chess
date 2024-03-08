#include <iostream>
#include <cassert>
#include <memory>
#include <string>

#include "uci.h"
#include "movepicker.h"

namespace uci {

std::vector<Option> options;
std::unique_ptr<Pool> pool;
std::unique_ptr<Pos>  pos;

std::string head;
std::vector<std::string> subcommands;

std::mutex print_mutex;
void print(std::string message) {
    print_mutex.lock();
    std::cout << message << std::endl;
    print_mutex.unlock();
}

std::string pop_subcommand() {
    
    if (!subcommands.size())
        return "";
    
    head = subcommands[0];
    subcommands.erase(subcommands.begin());
    
    return head;
}

std::string peak_subcommand() {
    
    if (!subcommands.size())
        return "";
    
    return subcommands[0];
}

void mainloop() {

    std::cout.setf(std::ios::unitbuf);
    
    pool = std::make_unique<Pool>(1, 64 * 1000 * 1000);
    pos  = std::make_unique<Pos >();

    pool->reset(*pos.get());

    handled_assert_out:
    await_command:
    
    std::string command;
    getline(std::cin, command);

    process_command:

    subcommands = split(command);

    process_subcommand:

    pop_subcommand();

    if (head == "uci") {
        std::cout << "id name Vision" << std::endl;
        std::cout << "id author Kenneth Wilber" << std::endl;

        for (Option& option : options) {
            std::cout << "option name " << option.name << " type " << option.type;
            if (option.type == "check" ) handled_assert(false);
            if (option.type == "spin"  ) handled_assert(false);
            if (option.type == "combo" ) handled_assert(false);
            if (option.type == "button") handled_assert(false);
            if (option.type == "string") handled_assert(false);
        }

        std::cout << "uciok" << std::endl;
    }
    else if (head == "debug") {
        handled_assert(false);
    }
    else if (head == "isready") {
        std::cout << "readyok" << std::endl;
    }
    else if (head == "setoption") {
        handled_assert(pop_subcommand() == "name");

        handled_assert(false);
    }
    else if (head == "register") {
        handled_assert(false);
    }
    else if (head == "ucinewgame") {
        pool->clear();
    }
    else if (head == "position") {
        pop_subcommand();
        if (head == "fen") {
            
            std::string fen = "";
            
            while (subcommands.size() && peak_subcommand() != "moves") {
                fen += pop_subcommand() + " ";
            }

            pos = std::make_unique<Pos>(fen);
        }
        else if (head == "startpos") {
            pos = std::make_unique<Pos>();
        }

        if (pop_subcommand() == "moves") {
            while (subcommands.size())
                handled_assert(pos->do_move(pop_subcommand()));
        }

        pool->reset(*pos.get());
        pool->tt->gen++;
    }
    else if (head == "go") {

        // reset limits
        pool->max_time = TIME_MAX;

        while (subcommands.size()) {

            pop_subcommand();

            if (head == "searchmoves") {
                handled_assert(false);
            }
            else if (head == "ponder") {
                pool->max_time = TIME_MAX;
            }
            else if (head == "wtime") {
                handled_assert(false);
            }
            else if (head == "btime") {
                handled_assert(false);
            }
            else if (head == "winc") {
                handled_assert(false);
            }
            else if (head == "binc") {
                handled_assert(false);
            }
            else if (head == "movestogo") {
                handled_assert(false);
            }
            else if (head == "depth") {
                handled_assert(false);
            }
            else if (head == "nodes") {
                handled_assert(false);
            }
            else if (head == "mate") {
                handled_assert(false);
            }
            else if (head == "movetime") {
                handled_assert(subcommands.size());
                pool->max_time = std::stoi(pop_subcommand());
            }
            else if (head == "infinite") {
                pool->max_time = TIME_MAX;
            }

        }

        pool->go();

    }
    else if (head == "stop") {
        pool->stop();
    }
    else if (head == "ponderhit") {
        handled_assert(false);
    }
    else if (head == "quit") {
        pool->stop();
        return;
    }
    else if (head == "d") {
        print(*pos.get(), true);
    }
    else if (head == "r") {
        Accumulator accumulator;
        accumulator.reset(*pos.get());
        Eval eval = nnue::evaluate(accumulator, *pos.get());
        std::cout << eval << std::endl;
    }
    else if (head == "m") {
        
        bool found;
        TTEntry* tt_entry = pool.get()->tt->probe(pos.get()->hashkey(), found);
        Move     tt_move  = tt_entry->move;

        MovePicker mp(pos.get(), tt_move, &pool.get()->threads[0]->history);
        
        while (mp.has_move()) {

            const Move  move  = mp.pop();
            const Score score = 0;
            
            std::cout << std::to_string(mp.stage) << " " << move_to_string(move) << " " << score << std::endl;

        }
    }


    goto await_command;
}

}









/*
#include "search.h"
#include "bits.h"
#include "types.h"
#include "pos.h"
#include "tt.h"
#include "movegen.h"
#include "uci.h"
#include <sstream>
#include <cassert>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>
#include <cstdlib>

namespace uci {

std::vector<Option> uci_options = {
    UCI_Option("Ponder", "check", "true", "option name Ponder type check default true"),
    UCI_Option("Hash", "spin", "512", "option name Hash type spin default 512 min 16 max 1024"),
    UCI_Option("OwnBook", "check", "false", "option name OwnBook type check default false"),
    UCI_Option("Threads", "spin", "1", "option name Threads type spin default 1 min 1 max 8"),
};

Option* get_uci_option(std::string name) {
    for (Option& uci_option : uci_options) {
        if (uci_option.name == name) return &uci_option;
    }
    return nullptr;
}

void mainloop() {
    std::cout.setf(ios::unitbuf);

    SearchInfo uci_si;
    
    std::string token;
    while (true) {
        loop_start_pre_cin:
        cin >> token;
        loop_start_post_cin:

        if (token == "uci") {
            std::cout << "id name VisionChess" << std::endl;
            std::cout << "id author Kenneth Wilber" << std::endl;

            for (Option option : uci_options) {
                std::cout << option.init << std::endl;
            }

            std::cout << "uciok" << std::endl;
        }
        else if (token == "isready") {
            std::cout << "readyok" << std::endl;
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
                std::cout << "no option named " << token << std::endl;
            }
        }
        else if (token == "ucinewgame") {
            uci_si.stop();
            uci_si.clear();
        }
        else if (token == "position") {
            std::string line;
            getline(cin, line);
            istringstream iss(line);
            
            iss >> token;
            if (token == "fen") {
                std::string part;
                std::string fen = "";
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
            std::string line;
            getline(cin, line);
            istringstream iss(line);

            Timestamp wtime = -1;
            Timestamp btime = -1;
            Timestamp winc =  0;
            Timestamp binc =  0;

            uci_si.ponder = false;
            uci_si.num_threads = stoi(get_uci_option("Threads")->value);
            uci_si.max_time = -1;
            uci_si.max_depth = DEPTH_MAX;
            uci_si.last_depth_searched = 0;
            bool time_is_perm = false;

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
                    uci_si.max_depth = DEPTH_MAX;
                    uci_si.max_time = -1;
                    time_is_perm = true;
                }
                else if (token == "perft") {
                    iss >> token;
                    int depth = stoi(token);
                    Pos pos_copy = uci_si.root_pos;
                    perft(pos_copy, depth, true);
                    goto loop_start_pre_cin;
                }
                else if (token == "movetime") {
                    iss >> token;
                    uci_si.max_time = stoi(token);
                    time_is_perm = true;
                }
            }

            if (!time_is_perm) uci_si.max_time = (uci_si.root_pos.turn == WHITE ? min((wtime / 30 + winc), wtime / 2) : min((btime / 30 + binc), btime / 2));
            bool verbose = true;
            thread(&SearchInfo::launch, &uci_si, ref(verbose)).detach();
            sleep_ms(10);
        }
        else if (token == "stop") {
            uci_si.stop();
        }
        else if (token == "ponderhit") {
            uci_si.start_time = get_current_ms();
            uci_si.ponder = false;
        }
        else if (token == "d") {
            print(uci_si.root_pos, true);
        }
        else if (token == "r") {
            Eval eval = eval_pos(uci_si.root_pos, EVAL_MIN, EVAL_MAX, true);
            std::cout << "Eval: " << eval_to_string(eval) << std::endl;
        }
        else if (token == "quit") {
            uci_si.stop();
            break;
        }
    }

}

} // namespace uci
*/