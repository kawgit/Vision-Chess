all: src/bits.o src/eval.o src/hash.o src/movegen.o src/order.o src/pos.o src/search.o src/timer.o src/tt.o src/types.o src/uci.o main
objects: src/bits.o src/eval.o src/hash.o src/movegen.o src/order.o src/pos.o src/search.o src/timer.o src/tt.o src/types.o src/uci.o

rel:
	clang++ -D NDEBUG -Ofast -pthread src/bits.o src/eval.o src/hash.o src/movegen.o src/order.o src/pos.o src/search.o src/timer.o src/tt.o src/types.o src/uci.o src/main.cpp -o main.exe
	clang++ -D NDEBUG -Ofast -pthread src/bits.o src/eval.o src/hash.o src/movegen.o src/order.o src/pos.o src/search.o src/timer.o src/tt.o src/types.o src/uci.o src/tests.cpp -o tests.exe

web: src/bits.o src/eval.o src/hash.o src/movegen.o src/order.o src/pos.o src/search.o src/timer.o src/tt.o src/types.o src/web.cpp
	em++ src/web.cpp src/bits.o src/eval.o src/hash.o src/movegen.o src/order.o src/pos.o src/search.o src/timer.o src/tt.o src/types.o -o web/Vision.js -s WASM=1 -sEXPORTED_RUNTIME_METHODS=cwrap -sEXPORTED_FUNCTIONS=_main,_process_uci_line,_web_worker,_web_stop,_probe_move,_probe_eval,_probe_depth,_probe_pv

main: src/bits.o src/eval.o src/hash.o src/movegen.o src/order.o src/pos.o src/search.o src/timer.o src/tt.o src/types.o src/uci.o src/main.cpp
	clang++ -Ofast -pthread src/bits.o src/eval.o src/hash.o src/movegen.o src/order.o src/pos.o src/search.o src/timer.o src/tt.o src/types.o src/uci.o src/main.cpp -o main.exe

tests: src/bits.o src/eval.o src/hash.o src/movegen.o src/order.o src/pos.o src/search.o src/timer.o src/tt.o src/types.o src/tests.cpp
	clang++ -Ofast -pthread src/bits.o src/eval.o src/hash.o src/movegen.o src/order.o src/pos.o src/search.o src/timer.o src/tt.o src/types.o src/tests.cpp -o tests.exe

src/bits.o: src/bits.cpp src/bits.h
	clang++ -c -Ofast src/bits.cpp -o src/bits.o
src/eval.o: src/eval.cpp src/eval.h
	clang++ -c -Ofast src/eval.cpp -o src/eval.o
src/hash.o: src/hash.cpp src/hash.h
	clang++ -c -Ofast src/hash.cpp -o src/hash.o
src/movegen.o: src/movegen.cpp src/movegen.h
	clang++ -c -Ofast src/movegen.cpp -o src/movegen.o
src/order.o: src/order.cpp src/order.h
	clang++ -c -Ofast src/order.cpp -o src/order.o
src/pos.o: src/pos.cpp src/pos.h
	clang++ -c -Ofast src/pos.cpp -o src/pos.o
src/search.o: src/search.cpp src/search.h
	clang++ -c -Ofast src/search.cpp -o src/search.o -pthread
src/timer.o: src/timer.cpp src/timer.h
	clang++ -c -Ofast src/timer.cpp -o src/timer.o
src/tt.o: src/tt.cpp src/tt.h
	clang++ -c -Ofast src/tt.cpp -o src/tt.o
src/types.o: src/types.cpp src/types.h
	clang++ -c -Ofast src/types.cpp -o src/types.o
src/uci.o: src/uci.cpp src/uci.h
	clang++ -c -Ofast -pthread src/uci.cpp -o src/uci.o

clean:
	del src/*.o
	del *.exe