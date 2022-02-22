#include <iostream>
#include "pos.h"
#include "bits.h"
#include "search.h"
#include "timer.h"
#include "search.h"
#include "types.h"
#include "movegen.h"
#include "eval.h"

using namespace std;

int MATCH_LENGTH = 10;

int main() {

	cout<<to_string(((float)(sizeof(NNUE)))/0x100000)<<endl;

	initMoveGen();
	initHash(39120);

	Pos startpos;
	print(startpos);
	NNUE champ;
	float champ_score = 0;
	Color champ_color = WHITE;

	for (int i = 0; i < 5; i++) {
		NNUE chall = champ;
		for (int m = 0; m < 1000; m++) chall.mutate(.11);

		return 0;
		champ.load_pos((char*)&startpos);
		chall.load_pos((char*)&startpos);

		for (int g = 0; g < MATCH_LENGTH; g++) {
			Pos p = startpos;

			SearchInfo s1;
			TT tt1;
			s1.tt = &tt1;
			
			SearchInfo s2;
			TT tt2;
			s2.tt = &tt2;

			while (!p.isGameOver()) {
				print(p);
				if (p.turn == champ_color) {
					p.nnue = &champ;
					search(p, 5, &s1);
					p.nnue = nullptr;

					bool found = false;
					p.makeMove(s1.tt->probe(p.hashkey, found)->move);
				}
				else {
					p.nnue = &chall;
					search(p, 5, &s2);
					p.nnue = nullptr;

					bool found = false;
					p.makeMove(s2.tt->probe(p.hashkey, found)->move);
				}
			}


			int result = p.getResult();
			if (result == 0) champ_score += .5;
			else if (result == 1 && champ_color == WHITE) champ_score++;
			else if (result == -1 && champ_color == BLACK) champ_score++;
			champ_color = getOppositeColor(champ_color);
		}

		if (champ_score < ((float)MATCH_LENGTH)/2) {
			champ = chall;
		}

		champ_score = 0;
		champ_color = WHITE;
	}

	champ.save("champ.nnue");

	return 0;
}