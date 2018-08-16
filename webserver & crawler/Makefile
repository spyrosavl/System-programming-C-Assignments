CC		= g++
FLAGS	= -c -w -std=gnu++11
LIB     =

all: server crawler
crawler: MyCrawler.o List.o
	$(CC) -pthread MyCrawler.o List.o -o mycrawler
server: Server.o List.o
	$(CC) -pthread Server.o List.o -o myhttpd
MyCrawler.o: MyCrawler.cpp
	$(CC) $(FLAGS) MyCrawler.cpp
Server.o: Server.cpp
	$(CC) $(FLAGS) Server.cpp
List.o: List.cpp
	$(CC) $(FLAGS) List.cpp


# clean up
clean:
	rm -f myhttpd Server.o List.o MyCrawler.o mycrawler
