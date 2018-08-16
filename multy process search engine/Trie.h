//
// Created by spyros on 5/3/2018.
//
#include "main.h"
#include "List.h"

#ifndef SEARCH_ENGINE_TRIE_H
#define SEARCH_ENGINE_TRIE_H


class Trie_Node {
public:
    Trie_Node(char c);
    ~Trie_Node();
    char character;
    List<TrieDataNode *> * DataNodes;
    List<Trie_Node *> * Children;
};

class Trie {
public:
    Trie();
    ~Trie();
    List<TrieDataNode *> * find_word_appearance(char *) const;
    int appearance_times(char * word) const ;
    void add_word(char *, int file_id, int line_id);
    List<SearchResult *> * search(char * query);
private:
    Trie_Node * root;
};
#endif //SEARCH_ENGINE_TRIE_H
