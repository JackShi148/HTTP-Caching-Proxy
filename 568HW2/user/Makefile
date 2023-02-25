all: user remote

user: user.cpp
	g++ -g -o user user.cpp
remote: remote.cpp
	g++ -g -o remote remote.cpp 

.PHONY:
	clean
clean:
	rm -rf *.o user remote
