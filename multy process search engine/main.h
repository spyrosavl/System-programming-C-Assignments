//
// Created by spyros on 4/4/2018.
//
#include <cstdlib>

#ifndef HW2_WORKER_MAIN_H
#define HW2_WORKER_MAIN_H

class StringIntPair{
public:
    int value;
    char * key;
};

class TrieDataNode{
public:
    TrieDataNode(char * w, int fid, int lid){
        file_id = fid;
        line_id = lid;
        word = w;
        times = 1;
    }
    ~TrieDataNode(){
        free(word);
    }
    int file_id;
    int line_id;
    int times;
    char * word;
};

class SearchResult{
public:
    SearchResult(int fid, int lid, int t){
        file_id = fid;
        line_id = lid;
        times = t;
    }
    int file_id, line_id, times;
};

#endif //HW2_WORKER_MAIN_H
