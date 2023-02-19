all: bits.o eval.o hash.o movegen.o order.o pos.o search.o timer.o tt.o types.o uci.o main
objects: bits.o eval.o hash.o movegen.o order.o pos.o search.o timer.o tt.o types.o uci.o

main: bits.o eval.o hash.o movegen.o order.o pos.o search.o timer.o tt.o types.o uci.o main.cpp
	em++ -o index.html -O3 -pthread main.cpp bits.o eval.o hash.o movegen.o order.o pos.o search.o timer.o tt.o types.o uci.o -sTOTAL_MEMORY=1079050240
	# use -sLLD_REPORT_UNDEFINED if you recieve error stuff is undefined

bits.o: bits.cpp bits.h
	em++ -c -O3 bits.cpp -o bits.o
eval.o: eval.cpp eval.h
	em++ -c -O3 eval.cpp -o eval.o
hash.o: hash.cpp hash.h
	em++ -c -O3 hash.cpp -o hash.o
movegen.o: movegen.cpp movegen.h
	em++ -c -O3 movegen.cpp -o movegen.o
order.o: order.cpp order.h
	em++ -c -O3 order.cpp -o order.o
pos.o: pos.cpp pos.h
	em++ -c -O3 pos.cpp -o pos.o
search.o: search.cpp search.h
	em++ -c -O3 search.cpp -o search.o -pthread
timer.o: timer.cpp timer.h
	em++ -c -O3 timer.cpp -o timer.o
tt.o: tt.cpp tt.h
	em++ -c -O3 tt.cpp -o tt.o
types.o: types.cpp types.h
	em++ -c -O3 types.cpp -o types.o
uci.o: uci.cpp uci.h
	em++ -c -O3 -pthread uci.cpp -o uci.o

clean:
	rm *.o
	rm *.exe