all: main

main: main.cpp server.cpp proxy.cpp client.cpp
	g++ -g  -o main main.cpp server.cpp proxy.cpp client.cpp -lpthread

.PHONY:
	clean
clean:
	rm -rf *.o main
