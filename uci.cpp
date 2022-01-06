#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include "pos.h"
#include "bits.h"
#include "search.h"
#include "timer.h"
#include "search.h"
#include "types.h"
#include "movegen.h"
#include "eval.h"

using namespace std;

int main() {
	initMoveGen();
	initHash(44324);
    Search s(Pos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
    s.num_threads = 1;
    string token;

    while (true) {
        cin>>token;

        tokenstart:
        if (token == "uci") {
            cout<<"id name VisionChess"<<endl;
            cout<<"id author Kenneth Wilber"<<endl;
            cout<<"uciok"<<endl;
        }
        else if (token == "debug") {
            
        }
        else if (token == "isready") {
            cout<<"readyok"<<endl;
        }
        else if (token == "setoption") {

        }
        else if (token == "register") {

        }
        else if (token == "ucinewgame") {
            s.tt.clear();
        }
        else if (token == "position") {
            s.tt.gen++;
            cin>>token;
            if (token == "startpos") s.root_pos = Pos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            else if (token == "fen") {
                string fen = "";
                for (int i = 0; i < 6; i++) {
                    cin>>token;
                    fen += token + " ";
                }
                s.root_pos = Pos(fen);
            }

            cin>>token;
            if (token == "moves") {
                while (true) {
                    cin>>token;
                    if (!s.root_pos.makeMove(token)) goto tokenstart;
                }
            }
            else goto tokenstart;
        }
        else if (token == "go") {
            s.ponder = false;
            s.infinite = false;
            s.go();
            while (true) {
                cin>>token;
                if (token == "ponder") {
                    s.ponder = true;
                }
                if (token == "searchmoves") {
                    //cout<<"SEARCHMOVES INCOMPLETE"<<endl;
                }
                if (token == "wtime") {
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
                    //cout<<"movestogo incomplete"<<endl;
                }
                else if (token == "depth") {
                    cin>>token;
                    s.max_depth = stoi(token);
                }
                else if (token == "nodes") {
                    //cout<<"nodes incomplete"<<endl;
                }
                else if (token == "mate") {
                    //cout<<"mate incomplete"<<endl;
                }
                else if (token == "movetime") {
                    //cout<<"movetime incomplete"<<endl;
                }
                else if (token == "infinite") {
                    s.infinite = true;
                }
                else goto tokenstart;
            }
        }
        else if (token == "stop") {
            s.stop();
        }
        else if (token == "ponderhit") {
            s.search_start = get_current_ms();
            while (get_time_diff(s.search_start) < 10) {};
            s.ponder = false;
        }
        else if (token == "quit") {
            s.stop();
            break;
        }
        else if (token == "d") {
            print(s.root_pos, true);
        }
    }

    cout<<"uci quit"<<endl;
}