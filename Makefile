all: bits.o eval.o hash.o movegen.o pos.o search.o timer.o tt.o types.o uci.exe

uci.exe: bits.o eval.o hash.o movegen.o pos.o search.o timer.o tt.o types.o uci.cpp
	g++ -Ofast bits.o eval.o hash.o movegen.o pos.o search.o timer.o tt.o types.o uci.cpp -o main.exe

bits.o: bits.cpp bits.h
	g++ -c -Ofast bits.cpp -o bits.o
eval.o: eval.cpp eval.h
	g++ -c -Ofast eval.cpp -o eval.o
hash.o: hash.cpp hash.h
	g++ -c -Ofast hash.cpp -o hash.o
movegen.o: movegen.cpp movegen.h
	g++ -c -Ofast movegen.cpp -o movegen.o
pos.o: pos.cpp pos.h
	g++ -c -Ofast pos.cpp -o pos.o
search.o: search.cpp search.h
	g++ -c -Ofast search.cpp -o search.o
timer.o: timer.cpp timer.h
	g++ -c -Ofast timer.cpp -o timer.o
tt.o: tt.cpp tt.h
	g++ -c -Ofast tt.cpp -o tt.o
types.o: types.cpp types.h
	g++ -c -Ofast types.cpp -o types.o

clean:
	del *.o
	del *.exe