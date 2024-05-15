#include <iostream>
#include <vector>
#include <string>
#include <thread>

#include "attacks.h"
#include "bits.h"
#include "move.h"
#include "movegen.h"
#include "pos.h"
#include "search.h"
#include "movepicker.h"
#include "util.h"
#include "uci.h"
#include "thread.h"



int main() {

	zobrist::init();
	attacks::init();

	uci::mainloop();

	return 0;
}