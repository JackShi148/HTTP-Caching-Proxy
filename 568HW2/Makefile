all: main

main: main.cpp server.cpp proxy.cpp
	g++ -g  -o main main.cpp server.cpp proxy.cpp -lpthread

.PHONY:
	clean
clean:
	rm -rf *.o main
