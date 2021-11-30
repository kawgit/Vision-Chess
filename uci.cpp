#include "uci.h"
#include "pos.h"
#include "types.h"
#include "search.h"
#include "movegen.h"
#include "util.h"
#include "zobrist.h"
#include <thread>
#include <iostream>
#include <vector>
#include <string>

int main() {
    initBB();
    initZ();
    initM();

    cout.setf(ios::unitbuf);
    bool running = true;

    Pos p;
    Search s(p);
    thread run_thread(&Search::runThread, s);

    while (running) {
        string token;
        cin>>token;
        token_start:
        if (token == "uci") {
            cout<<"id name VisionChess author Kenneth Wilber"<<endl;
            cout<<"uciok"<<endl;
        }
        else if (token == "debug") {
            //nothing yet ¯\_(ツ)_/¯
        }
        else if (token == "isready") {
            cout<<"readyok"<<endl;
        }
        else if (token == "setoption") {
            //nothing yet ¯\_(ツ)_/¯
        }
        else if (token == "register") {
            //i dont understand this field ¯\_(ツ)_/¯
        }
        else if (token == "ucinewgame") {
            s.table.clear();
        }
        else if (token == "position") {
            cin>>token;
            if (token == "startpos") {
                p = Pos();
            }
            else if (token == "fen") {
                cin>>token;
                p = Pos(token);
            }
            else goto token_start;
            
            cin>>token;
            if (token == "moves") {
                bool found = true;
                while (found) {
                    cin>>token;
                    found = false;
                    vector<Move> moves;
                    addLegalMoves(p, moves);
                    for (Move &m : moves) {
                        if (token == m.getSAN()) {
                            p.makeMove(m);
                            found = true;
                            break;
                        }
                    }
                }
                goto token_start;
            }
            else goto token_start;
        }
        else if (token == "go") {
            s.searching = true;
            while (true) {
                cin>>token;
                if (token == "searchmoves") {
                    //tbd ¯\_(ツ)_/¯
                }
                else if (token == "ponder") {
                    s.pondering = true;
                }
                else if (token == "wtime") {
                    cin>>token;
                    s.wtime = stoi(token);
                }
                else if (token == "btime") {
                    cin>>token;
                    s.btime = stoi(token);
                }
                else if (token == "winc") {
                    cin>>token;
                    s.winc = stoi(token);
                }
                else if (token == "binc") {
                    cin>>token;
                    s.binc = stoi(token);
                }
                else if (token == "movestogo") {
                    // tbd ¯\_(ツ)_/¯
                }
                else if (token == "depth") {
                    cin>>token;
                    s.max_depth = stoi(token);
                }
                else if (token == "nodes") {
                    // tbd ¯\_(ツ)_/¯
                }
                else if (token == "mate") {
                    // tbd ¯\_(ツ)_/¯
                }
                else if (token == "movetime") {
                    // tbd ¯\_(ツ)_/¯
                }
                else if (token == "infinite") {
                    s.infinite = true;
                }
                else break;
            }
            goto token_start;
        }
        else if (token == "stop") {
            s.stop();
        }
        else if (token == "ponderhit") {
            s.pondering = false;
        }
        else if (token == "quit") {
            s.quit();
            running = false;
        }
        else if (token == "d") {
            print(p);
        }
        else {
            cout<<"Unknown token"<<endl;
        }
    }

    cout<<"waiting for run_thread join to end"<<endl;
    run_thread.join();
}