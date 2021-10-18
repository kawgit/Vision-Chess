all : pos.o types.o util.o zobrist.o movegen.o main.exe

main.exe : main.cpp pos.o types.o util.o zobrist.o movegen.o 
	g++ -Ofast -o main.exe main.cpp pos.o types.o util.o zobrist.o movegen.o

pos.o : pos.cpp pos.h types.h util.h
	g++ -c -Ofast -o pos.o pos.cpp

types.o : types.cpp types.h
	g++ -c -Ofast -o types.o types.cpp

util.o : util.cpp util.h
	g++ -c -Ofast -o util.o util.cpp

zobrist.o : zobrist.cpp zobrist.h
	g++ -c -Ofast -o zobrist.o zobrist.cpp

movegen.o : movegen.cpp movegen.h
	g++ -c -Ofast -o movegen.o movegen.cpp

clean : 
	del *.o