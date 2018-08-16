//
// Created by spyros on 28/2/2018.
//

#include "List.h"
#include "Trie.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <cstring>

template class List<char *>;
template class List<int>;
template class List<StringIntPair *>;
template class List<Trie_Node*>;
template class List<TrieDataNode*>;
template class List<List<TrieDataNode * > *>;
template class List<SearchResult *>;

template <class Items>
List<Items>::List(int initial_size) {
    assert(initial_size>0);
    this->Array = (Items*)malloc(initial_size*sizeof(Items));
    this->allocated = initial_size;
    this->len=0;
    this->noDeleteNodes = 0;
    this->start_from=0;
}
template <class Items>
List<Items>::List(int initial_size, int noDeleteNodes) {
    assert(initial_size>0);
    this->Array = (Items*)malloc(initial_size*sizeof(Items));
    this->allocated = initial_size;
    this->len=0;
    this->noDeleteNodes = noDeleteNodes;
    this->start_from=0;
}

template <class Items>
void List<Items>::appendElement(Items item){
    if(len>=allocated){
        this->allocate_more();
    }
    this->len++;
    Array[len-1] = item;
    assert(Array[len-1]!=NULL);
}

template <>
void List<int>::appendElement(int item){
    if(len>=allocated){
        this->allocate_more();
    }
    this->len++;
    Array[len-1] = item;
}

template <class Items>
List<Items>::~List() {
    for (int i = 0; i < len; ++i) {
        if(Array[i]!=NULL){
            if(!this->noDeleteNodes){
                delete Array[i]; //free tin domi pou evala mesa
            }
            Array[i] = NULL;
        }
    }
    free(Array);
}

template <>
List<char *>::~List() {
    for (int i = 0; i < len; ++i) {
        if(Array[i]!=NULL){
            if(!this->noDeleteNodes)free((char *)Array[i]); //free tin domi pou evala mesa
            Array[i] = NULL;
        }
    }
    free(Array);
}

template <>
List<int>::~List() {
    free(Array);
}

template <class Items>
bool List<Items>::exists(Items query){
    for (int i = 0; i < this->getLen(); ++i) {
        if(this->value(i) == query)return true;
    }
    return false;
}

template <class Items>
Items &List<Items>::operator[](int i) const {
    int index = i - this->start_from;
    assert(index<len && index>=0);
    return Array[index];
}

template <class Items>
Items List<Items>::value(int i) const {
    int index = i - this->start_from;
    if(!(index<len && index>=0)){
        printf("i=%d index=%d len=%d\n", i, index, len);
    }
    assert(index<len && index>=0);
    return Array[index];
}

template <class Items>
void List<Items>::update(int i, Items item){
    assert(i<len && i>=0);
    Array[i] = item ;
}

template <class Items>
int List<Items>::getLen() const {
    return len;
}

template <class Items>
void List<Items>::allocate_more(){
    this->allocated = int(this->allocated * 1.5);
    this->Array = (Items*) realloc (this->Array, this->allocated * sizeof(Items));
    assert(this->Array!=NULL);
}