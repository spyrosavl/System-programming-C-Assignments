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
    int appearance_times(char * word, int id) const ;
    void add_word(char *, int);
    List<List<TrieDataNode * > *> * all_words_appearances();
    void all_words_appearances_rec(Trie_Node * root, List<List<TrieDataNode * > *> * Result_List);
    List<SearchResult *> * search(char * query, List<char *> * texts, List<int> * stringsWordsCounter, double AvgWords);
private:
    Trie_Node * root;
    double docScore(char * query, char * doc, int doc_id, int wordsNo, double AvgWords, int NoOfDocs);
};
#endif //SEARCH_ENGINE_TRIE_H
