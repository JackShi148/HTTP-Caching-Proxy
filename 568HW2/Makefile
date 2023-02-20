all: main

<<<<<<< HEAD
main: main.cpp client.cpp response.cpp request.cpp
	g++ -g  -o main main.cpp client.cpp response.cpp request.cpp -lpthread
# test: test.cpp proxy.cpp log.cpp cache.cpp request.cpp response.cpp server.cpp
# 	g++ -g  -o test test.cpp proxy.cpp log.cpp cache.cpp request.cpp response.cpp server.cpp -lpthread

=======
main: main.cpp server.cpp proxy.cpp client.cpp request.cpp log.cpp
	g++ -g  -o main main.cpp server.cpp proxy.cpp client.cpp request.cpp log.cpp -lpthread
>>>>>>> 34de596ca9c29ffe891bf6d0f0de572890a5d235

.PHONY:
	clean
clean:
	rm -rf *.o main