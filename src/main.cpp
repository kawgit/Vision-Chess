#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "attacks.h"
#include "bits.h"
#include "move.h"
#include "movegen.h"
#include "movepicker.h"
#include "pos.h"
#include "search.h"
#include "thread.h"
#include "uci.h"
#include "util.h"

int main() {

	zobrist::init();
	attacks::init();

	uci::mainloop();

	return 0;
}