ASM_BUILD_DEST = build/asm
WASM_BUILD_DEST = build/wasm

all: initdir $(ASM_BUILD_DEST)/bits.o $(ASM_BUILD_DEST)/eval.o $(ASM_BUILD_DEST)/hash.o $(ASM_BUILD_DEST)/movegen.o $(ASM_BUILD_DEST)/order.o $(ASM_BUILD_DEST)/pos.o $(ASM_BUILD_DEST)/search.o $(ASM_BUILD_DEST)/timer.o $(ASM_BUILD_DEST)/tt.o $(ASM_BUILD_DEST)/types.o $(ASM_BUILD_DEST)/uci.o main.exe

initdir:
	-mkdir build
	-mkdir $(ASM_BUILD_DEST)
	-mkdir $(WASM_BUILD_DEST)

main.exe: initdir $(ASM_BUILD_DEST)/bits.o $(ASM_BUILD_DEST)/eval.o $(ASM_BUILD_DEST)/hash.o $(ASM_BUILD_DEST)/movegen.o $(ASM_BUILD_DEST)/order.o $(ASM_BUILD_DEST)/pos.o $(ASM_BUILD_DEST)/search.o $(ASM_BUILD_DEST)/timer.o $(ASM_BUILD_DEST)/tt.o $(ASM_BUILD_DEST)/types.o $(ASM_BUILD_DEST)/uci.o src/main.cpp
	clang++ -Ofast -pthread  -o main.exe src/main.cpp $(ASM_BUILD_DEST)/bits.o $(ASM_BUILD_DEST)/eval.o $(ASM_BUILD_DEST)/hash.o $(ASM_BUILD_DEST)/movegen.o $(ASM_BUILD_DEST)/order.o $(ASM_BUILD_DEST)/pos.o $(ASM_BUILD_DEST)/search.o $(ASM_BUILD_DEST)/timer.o $(ASM_BUILD_DEST)/tt.o $(ASM_BUILD_DEST)/types.o $(ASM_BUILD_DEST)/uci.o

tests.exe: initdir $(ASM_BUILD_DEST)/bits.o $(ASM_BUILD_DEST)/eval.o $(ASM_BUILD_DEST)/hash.o $(ASM_BUILD_DEST)/movegen.o $(ASM_BUILD_DEST)/order.o $(ASM_BUILD_DEST)/pos.o $(ASM_BUILD_DEST)/search.o $(ASM_BUILD_DEST)/timer.o $(ASM_BUILD_DEST)/tt.o $(ASM_BUILD_DEST)/types.o src/tests.cpp
	clang++ -Ofast -pthread -o tests.exe src/tests.cpp $(ASM_BUILD_DEST)/bits.o $(ASM_BUILD_DEST)/eval.o $(ASM_BUILD_DEST)/hash.o $(ASM_BUILD_DEST)/movegen.o $(ASM_BUILD_DEST)/order.o $(ASM_BUILD_DEST)/pos.o $(ASM_BUILD_DEST)/search.o $(ASM_BUILD_DEST)/timer.o $(ASM_BUILD_DEST)/tt.o $(ASM_BUILD_DEST)/types.o

web: initdir $(WASM_BUILD_DEST)/bits.o $(WASM_BUILD_DEST)/eval.o $(WASM_BUILD_DEST)/hash.o $(WASM_BUILD_DEST)/movegen.o $(WASM_BUILD_DEST)/order.o $(WASM_BUILD_DEST)/pos.o $(WASM_BUILD_DEST)/search.o $(WASM_BUILD_DEST)/timer.o $(WASM_BUILD_DEST)/tt.o $(WASM_BUILD_DEST)/types.o $(WASM_BUILD_DEST)/uci.o src/main.cpp
	em++ -pthread -sTOTAL_MEMORY=1310720000 -o web.html src/tests.cpp $(WASM_BUILD_DEST)/bits.o $(WASM_BUILD_DEST)/eval.o $(WASM_BUILD_DEST)/hash.o $(WASM_BUILD_DEST)/movegen.o $(WASM_BUILD_DEST)/order.o $(WASM_BUILD_DEST)/pos.o $(WASM_BUILD_DEST)/search.o $(WASM_BUILD_DEST)/timer.o $(WASM_BUILD_DEST)/tt.o $(WASM_BUILD_DEST)/types.o $(WASM_BUILD_DEST)/uci.o

$(ASM_BUILD_DEST)/bits.o: src/bits.cpp src/bits.h
	clang++ -c -Ofast src/bits.cpp -o $(ASM_BUILD_DEST)/bits.o
$(ASM_BUILD_DEST)/eval.o: src/eval.cpp src/eval.h
	clang++ -c -Ofast src/eval.cpp -o $(ASM_BUILD_DEST)/eval.o
$(ASM_BUILD_DEST)/hash.o: src/hash.cpp src/hash.h
	clang++ -c -Ofast src/hash.cpp -o $(ASM_BUILD_DEST)/hash.o
$(ASM_BUILD_DEST)/movegen.o: src/movegen.cpp src/movegen.h
	clang++ -c -Ofast src/movegen.cpp -o $(ASM_BUILD_DEST)/movegen.o
$(ASM_BUILD_DEST)/order.o: src/order.cpp src/order.h
	clang++ -c -Ofast src/order.cpp -o $(ASM_BUILD_DEST)/order.o
$(ASM_BUILD_DEST)/pos.o: src/pos.cpp src/pos.h
	clang++ -c -Ofast src/pos.cpp -o $(ASM_BUILD_DEST)/pos.o
$(ASM_BUILD_DEST)/search.o: src/search.cpp src/search.h
	clang++ -c -Ofast src/search.cpp -o $(ASM_BUILD_DEST)/search.o -pthread
$(ASM_BUILD_DEST)/timer.o: src/timer.cpp src/timer.h
	clang++ -c -Ofast src/timer.cpp -o $(ASM_BUILD_DEST)/timer.o
$(ASM_BUILD_DEST)/tt.o: src/tt.cpp src/tt.h
	clang++ -c -Ofast src/tt.cpp -o $(ASM_BUILD_DEST)/tt.o
$(ASM_BUILD_DEST)/types.o: src/types.cpp src/types.h
	clang++ -c -Ofast src/types.cpp -o $(ASM_BUILD_DEST)/types.o
$(ASM_BUILD_DEST)/uci.o: src/uci.cpp src/uci.h
	clang++ -c -Ofast -pthread src/uci.cpp -o $(ASM_BUILD_DEST)/uci.o

$(WASM_BUILD_DEST)/bits.o: src/bits.cpp src/bits.h
	em++ -c -O3 src/bits.cpp -o $(WASM_BUILD_DEST)/bits.o
$(WASM_BUILD_DEST)/eval.o: src/eval.cpp src/eval.h
	em++ -c -O3 src/eval.cpp -o $(WASM_BUILD_DEST)/eval.o
$(WASM_BUILD_DEST)/hash.o: src/hash.cpp src/hash.h
	em++ -c -O3 src/hash.cpp -o $(WASM_BUILD_DEST)/hash.o
$(WASM_BUILD_DEST)/movegen.o: src/movegen.cpp src/movegen.h
	em++ -c -O3 src/movegen.cpp -o $(WASM_BUILD_DEST)/movegen.o
$(WASM_BUILD_DEST)/order.o: src/order.cpp src/order.h
	em++ -c -O3 src/order.cpp -o $(WASM_BUILD_DEST)/order.o
$(WASM_BUILD_DEST)/pos.o: src/pos.cpp src/pos.h
	em++ -c -O3 src/pos.cpp -o $(WASM_BUILD_DEST)/pos.o
$(WASM_BUILD_DEST)/search.o: src/search.cpp src/search.h
	em++ -c -O3 src/search.cpp -o $(WASM_BUILD_DEST)/search.o -pthread
$(WASM_BUILD_DEST)/timer.o: src/timer.cpp src/timer.h
	em++ -c -O3 src/timer.cpp -o $(WASM_BUILD_DEST)/timer.o
$(WASM_BUILD_DEST)/tt.o: src/tt.cpp src/tt.h
	em++ -c -O3 src/tt.cpp -o $(WASM_BUILD_DEST)/tt.o
$(WASM_BUILD_DEST)/types.o: src/types.cpp src/types.h
	em++ -c -O3 src/types.cpp -o $(WASM_BUILD_DEST)/types.o
$(WASM_BUILD_DEST)/uci.o: src/uci.cpp src/uci.h
	em++ -c -O3 -pthread src/uci.cpp -o $(WASM_BUILD_DEST)/uci.o

clean:
	echo $(SHELL)
	-rm $(ASM_BUILD_DEST)/*.o
	-rm $(WASM_BUILD_DEST)/*.o
	-rm *.exe