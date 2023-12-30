ASM_BUILD_DEST = build
CXX_FLAGS = -Ofast -DNDEBUG -std=c++20
CXX_COMPILER = g++

all: initdir build/bits.o build/eval.o build/hash.o build/move.o build/movegen.o build/order.o build/pos.o build/search.o build/timer.o build/tt.o build/types.o build/uci.o Vision

initdir:
	-mkdir build

Vision: build/bits.o build/eval.o build/hash.o build/move.o build/movegen.o build/order.o build/pos.o build/search.o build/timer.o build/tt.o build/types.o build/uci.o src/main.cpp
	$(CXX_COMPILER) $(CXX_FLAGS) -pthread -o Vision.exe src/main.cpp build/*.o

tests: build/bits.o build/eval.o build/hash.o build/move.o build/movegen.o build/order.o build/pos.o build/search.o build/timer.o build/tt.o build/types.o src/tests.cpp
	$(CXX_COMPILER) $(CXX_FLAGS) -pthread -o tests.exe src/tests.cpp build/*.o

build/bits.o: src/bits.cpp src/bits.h
	$(CXX_COMPILER) -c $(CXX_FLAGS) src/bits.cpp -o build/bits.o
build/eval.o: src/eval.cpp src/eval.h
	$(CXX_COMPILER) -c $(CXX_FLAGS) src/eval.cpp -o build/eval.o
build/hash.o: src/hash.cpp src/hash.h
	$(CXX_COMPILER) -c $(CXX_FLAGS) src/hash.cpp -o build/hash.o
build/move.o: src/move.cpp src/move.h
	$(CXX_COMPILER) -c $(CXX_FLAGS) src/move.cpp -o build/move.o
build/movegen.o: src/movegen.cpp src/movegen.h
	$(CXX_COMPILER) -c $(CXX_FLAGS) src/movegen.cpp -o build/movegen.o
build/order.o: src/order.cpp src/order.h
	$(CXX_COMPILER) -c $(CXX_FLAGS) src/order.cpp -o build/order.o
build/pos.o: src/pos.cpp src/pos.h
	$(CXX_COMPILER) -c $(CXX_FLAGS) src/pos.cpp -o build/pos.o
build/search.o: src/search.cpp src/search.h
	$(CXX_COMPILER) -c $(CXX_FLAGS) src/search.cpp -o build/search.o -pthread
build/timer.o: src/timer.cpp src/timer.h
	$(CXX_COMPILER) -c $(CXX_FLAGS) src/timer.cpp -o build/timer.o
build/tt.o: src/tt.cpp src/tt.h
	$(CXX_COMPILER) -c $(CXX_FLAGS) src/tt.cpp -o build/tt.o
build/types.o: src/types.cpp src/types.h
	$(CXX_COMPILER) -c $(CXX_FLAGS) src/types.cpp -o build/types.o
build/uci.o: src/uci.cpp src/uci.h
	$(CXX_COMPILER) -c $(CXX_FLAGS) -pthread src/uci.cpp -o build/uci.o

clean:
	@-rm build/*.o
	@-del build\*.o