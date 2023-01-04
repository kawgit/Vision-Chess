all: bits.o eval.o hash.o movegen.o order.o pos.o search.o timer.o tt.o types.o uci.o main.exe unit_tests.exe
objects: bits.o eval.o hash.o movegen.o order.o pos.o search.o timer.o tt.o types.o uci.o

main.exe: bits.o eval.o hash.o movegen.o order.o pos.o search.o timer.o tt.o types.o uci.o main.cpp
	clang++ -Ofast -pthread bits.o eval.o hash.o movegen.o order.o pos.o search.o timer.o tt.o types.o uci.o main.cpp -o main.exe

unit_tests.exe: bits.o eval.o hash.o movegen.o order.o pos.o search.o timer.o tt.o types.o uci.o unit_tests.cpp
	clang++ -Ofast -pthread bits.o eval.o hash.o movegen.o order.o pos.o search.o timer.o tt.o types.o uci.o unit_tests.cpp -o unit_tests.exe

bits.o: bits.cpp bits.h
	clang++ -c -Ofast bits.cpp -o bits.o
eval.o: eval.cpp eval.h
	clang++ -c -Ofast eval.cpp -o eval.o
hash.o: hash.cpp hash.h
	clang++ -c -Ofast hash.cpp -o hash.o
movegen.o: movegen.cpp movegen.h
	clang++ -c -Ofast movegen.cpp -o movegen.o
order.o: order.cpp order.h
	clang++ -c -Ofast order.cpp -o order.o
pos.o: pos.cpp pos.h
	clang++ -c -Ofast pos.cpp -o pos.o
search.o: search.cpp search.h
	clang++ -c -Ofast search.cpp -o search.o -pthread
timer.o: timer.cpp timer.h
	clang++ -c -Ofast timer.cpp -o timer.o
tt.o: tt.cpp tt.h
	clang++ -c -Ofast tt.cpp -o tt.o
types.o: types.cpp types.h
	clang++ -c -Ofast types.cpp -o types.o
uci.o: uci.cpp uci.h
	clang++ -c -Ofast -pthread uci.cpp -o uci.o

clean:
	del *.o