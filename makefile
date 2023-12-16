myshell: main.o ls.o search.o signal.o cd.o pwd.o history.o pinfo.o
	g++ -w main.o ls.o search.o signal.o cd.o pwd.o history.o pinfo.o -o shell

main.o: main.cpp
	g++ -w -c main.cpp

ls.o: ls.cpp headers.h
	g++ -w -c ls.cpp

search.o: search.cpp headers.h
	g++ -w -c search.cpp

cd.o: cd.cpp headers.h
	g++ -w -c cd.cpp

pwd.o: pwd.cpp headers.h
	g++ -w -c pwd.cpp

signal.o: signal.cpp headers.h
	g++ -w -c signal.cpp

history.o: history.cpp headers.h
	g++ -w -c history.cpp

pinfo.o: pinfo.cpp headers.h
	g++ -w -c pinfo.cpp

clean:
	rm -f *.o shell