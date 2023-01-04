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
#include "uci.h"

using namespace std;


int main(int argc, char* argv[]) {
	initHash(4);
	initMoveGen();

	uci();

}