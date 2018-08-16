//
// Created by spyros on 3/3/2018.
//
#include <cstdlib>
#include <cstdio>

#ifndef SEARCH_ENGINE_MAIN_H
#define SEARCH_ENGINE_MAIN_H
class StringIntPair{
public:
    int value;
    char * key;
};

class TrieDataNode{
public:
    TrieDataNode(char * w, int id, int t){
        doc_id = id;
        times = t;
        word = w;
    }
    ~TrieDataNode(){
        free(word);
    }
    int doc_id;
    int times;
    char * word;
};


class SearchResult{
public:
    SearchResult(int id, char * q, double s){
        document_id = id;
        query = q;
        score = s;
    }
    double score;
    char * query;
    int document_id;
};

#endif //SEARCH_ENGINE_MAIN_H
