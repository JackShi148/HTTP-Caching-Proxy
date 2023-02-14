all: main

main: main.cpp client.cpp response.cpp request.cpp
	g++ -g  -o main main.cpp client.cpp response.cpp request.cpp -lpthread
# test: test.cpp proxy.cpp log.cpp cache.cpp request.cpp response.cpp server.cpp
# 	g++ -g  -o test test.cpp proxy.cpp log.cpp cache.cpp request.cpp response.cpp server.cpp -lpthread


.PHONY:
	clean
clean:
	rm -rf *.o main