//
// Created by spyros on 5/3/2018.
//
#include "Trie.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <cmath>

Trie_Node::Trie_Node(char c){
    this->character = c;
    this->Children = new List<Trie_Node *>(10);
    this->DataNodes = new List<TrieDataNode *>(10);
}
Trie_Node::~Trie_Node(){
    delete Children;
    delete DataNodes;
}

Trie::Trie(){
    this->root = new Trie_Node((char)NULL);
}
Trie::~Trie(){
    delete root;
}
List<TrieDataNode *> * Trie::find_word_appearance(char * word) const{
    Trie_Node * current_trie_node = this->root;
    Trie_Node * next_node = NULL ;
    while ( *word != '\0' ) {
        next_node = NULL;
        for (int i = 0; i < current_trie_node->Children->getLen(); ++i) {
            if(current_trie_node->Children->value(i)->character == *word){
                next_node=current_trie_node->Children->value(i);
                break;
            }
        }
        if(!next_node){
            return NULL;
        }
        current_trie_node = next_node;
        word++; // move to next position
    }
    return  current_trie_node->DataNodes;
}
void Trie::add_word(char * word, int file_id, int line_id){
    char * whole_word = word;
    Trie_Node * current_trie_node = this->root;
    Trie_Node * next_node = NULL ;
    while ( *word != '\0' ) {
        next_node = NULL;
        //printf ("%c \n", *word  );
        for (int i = 0; i < current_trie_node->Children->getLen(); ++i) {
            if(current_trie_node->Children->value(i)->character == *word){
                next_node=current_trie_node->Children->value(i);
                break;
            }
        }
        if(!next_node){
            next_node = new Trie_Node(*word);
            current_trie_node->Children->appendElement(next_node);
        }
        current_trie_node = next_node;
        word++; // move to next position
    }
    for (int j = 0; j < current_trie_node->DataNodes->getLen(); ++j) {
        if(current_trie_node->DataNodes->value(j)->file_id == file_id && current_trie_node->DataNodes->value(j)->line_id == line_id){
            current_trie_node->DataNodes->value(j)->times++;
            //printf("%s\n",current_trie_node->DataNodes->value(j)->word);
            return;
        }
    }
    char * copy_word = (char *)malloc(strlen(whole_word)+1);
    strcpy(copy_word, whole_word);
    current_trie_node->DataNodes->appendElement(new TrieDataNode(copy_word, file_id, line_id));
}

int Trie::appearance_times(char * word) const {
    int count=0;
    List<TrieDataNode *> * result = find_word_appearance(word);
    if(result){
        for (int i = 0; i < result->getLen(); ++i) {
            count+=result->value(i)->times;
        }
    }
    return  count;
}



List<SearchResult *> * Trie::search(char * query){
    List<SearchResult *> * results = new List<SearchResult *>(20);
    List<int> * pairsInResult = new List<int>(20);
    char * text_to_break = (char *)malloc(strlen(query)+1);
    strcpy(text_to_break, query);
    char * pch = strtok (text_to_break," \t");

    while (pch != NULL){ //for each word in query Until 10
        //printf("%s\n", pch);
        List<TrieDataNode *> * wordApp = this->find_word_appearance(pch);
        if(!wordApp){
            pch = strtok(NULL, " \t");
            continue;
        }
        for(int i = 0; i < wordApp->getLen(); ++i){
            int file_id = wordApp->value(i)->file_id;
            int line_id = wordApp->value(i)->line_id;
            int times = wordApp->value(i)->times;
            int unique_cantor_pair = (0.5)*(file_id+line_id)*(file_id+line_id+1)+line_id;

            if(!pairsInResult->exists(unique_cantor_pair)){
                pairsInResult->appendElement(unique_cantor_pair);
                results->appendElement(new SearchResult(file_id, line_id, times));
            }
        }
        //go to next
        pch = strtok(NULL, " \t");
    }
    free(text_to_break);
    delete pairsInResult;
    return  results;
}
