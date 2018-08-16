OBJS	= main.o List.o Trie.o util.o
OUT 	= search
CC		= g++
FLAGS	= -c -O3 -std=gnu++98
LIB     =

$(OUT): $(OBJS)
	$(CC) $(OBJS) -o $(OUT)
main.o: main.cpp
	$(CC) $(FLAGS) main.cpp
List.o: List.cpp
	$(CC) $(FLAGS) List.cpp
Trie.o: Trie.cpp
	$(CC) $(FLAGS) Trie.cpp
util.o: util.cpp
	$(CC) $(FLAGS) util.cpp

# clean up
clean:
	rm -f $(OBJS) $(OUT)
