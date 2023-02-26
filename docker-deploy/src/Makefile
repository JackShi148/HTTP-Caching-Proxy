all: main

main: main.cpp server.cpp proxy.cpp client.cpp request.cpp log.cpp response.cpp cache.cpp
	g++ -g -ggdb3 -o main main.cpp server.cpp proxy.cpp client.cpp request.cpp log.cpp response.cpp cache.cpp -lpthread

.PHONY:
	clean
clean:
	rm -rf *.o main
