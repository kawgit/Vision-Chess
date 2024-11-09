#include <cassert>
#include <iostream>
#include <memory>
#include <string>

#include "movepicker.h"
#include "uci.h"

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
        print_pos(*pos.get(), true);
    }
    else if (head == "r") {
        Evaluator evaluator;
        evaluator.reset(*pos.get());
        Eval eval = evaluator.evaluate(*pos.get());
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

    if (!subcommands.size())
        goto await_command;
    else
        goto process_command;
}

}