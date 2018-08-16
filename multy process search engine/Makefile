CC		= g++
FLAGS	= -c -O3 -w -std=gnu++98
LIB     =

all: worker jobExecutor
worker: worker_main.o List.o Trie.o
	$(CC) worker_main.o List.o Trie.o -o worker
jobExecutor: jobExecutor_main.o List.o Trie.o
	$(CC) jobExecutor_main.o List.o Trie.o -o jobExecutor
worker_main.o: worker_main.cpp
	$(CC) $(FLAGS) worker_main.cpp
jobExecutor_main.o: jobExecutor_main.cpp
	$(CC) $(FLAGS) jobExecutor_main.cpp
List.o: List.cpp
	$(CC) $(FLAGS) List.cpp
Trie.o: Trie.cpp
	$(CC) $(FLAGS) Trie.cpp

# clean up
clean:
	rm -f worker jobExecutor worker_main.o List.o Trie.o jobExecutor.o
