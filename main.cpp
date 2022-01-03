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

vector<string> split(string x, char delim = ' ')
{
    x += delim; //includes a delimiter at the end so last word is also read
    vector<string> splitted;
    string temp = "";
    for (int i = 0; i < x.length(); i++)
    {
        if (x[i] == delim)
        {
            splitted.push_back(temp); //store words in "splitted" vector
            temp = "";
            i++;
        }
        temp += x[i];
    }
    return splitted;
}

int main() {
	initMoveGen();
	initHash(323);

	Search s(Pos("r2q2k1/5brp/1pQ5/5N2/3P3b/1P2P3/P2B1PP1/R4RK1 b - - 4 34"));
	s.wtime = 3000;
	s.btime = 3000;
	s.binc = 1000;
	s.binc = 1000;
	s.num_threads = 4;

	Pos& p = s.root_pos;

	print(p);

	while (!p.isGameOver()) {
	
	/*
		{
			string move;
			cin>>move;

			p.makeMove(move);
			print(p);
			cout<<getFen(p)<<endl;

			if (p.isGameOver()) break;
		}
*/
		{
			s.tt.gen++;
			s.go();
			Move bestmove = s.tt.getPV(s.root_pos)[0];

			p.makeMove(bestmove);
			print(p);
			print(p.move_log); cout<<endl;
			cout<<getFen(p)<<endl;

			if (p.isGameOver()) break;
		}
	}

	print(p);
	cout<<getFen(p)<<endl;
	print(p.move_log);
	cout<<"end main reached"<<endl;
	return 0;
}