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
#include "util.h"

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
        //printf ("%c \n", *word  );
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
void Trie::add_word(char * word, int doc_id){
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
        if(current_trie_node->DataNodes->value(j)->doc_id == doc_id){
            current_trie_node->DataNodes->value(j)->times++;
            return;
        }
    }
    //printf("%s\n", whole_word);
    char * copy_word = (char *)malloc(strlen(whole_word)+1);
    strcpy(copy_word, whole_word);
    current_trie_node->DataNodes->appendElement(new TrieDataNode(copy_word, doc_id, 1));
}

int Trie::appearance_times(char * word, int id) const {
    int count=0;
    List<TrieDataNode *> * result = find_word_appearance(word);
    if(result){
        for (int i = 0; i < result->getLen(); ++i) {
            if(id<0 || (result->value(i)->doc_id == id)){
                count+=result->value(i)->times;
            }
        }
    }
    return  count;
}


List<List<TrieDataNode * > *> * Trie::all_words_appearances(){
    List<List<TrieDataNode * > *> * Result = new List<List<TrieDataNode * > *>(20,1);
    this->all_words_appearances_rec(this->root, Result);
    return  Result;
}


void Trie::all_words_appearances_rec(Trie_Node * root, List<List<TrieDataNode * > *> * Result_List){
    if(root->DataNodes->getLen()>0){
        Result_List->appendElement(root->DataNodes);
    }
    for (int i = 0; i < root->Children->getLen(); ++i) {
        this->all_words_appearances_rec(root->Children->value(i), Result_List);
    }
}

List<SearchResult *> * Trie::search(char * query, List<char *> * texts, List<int> * stringsWordsCounter, double AvgWords){
    List<SearchResult *> * results = new List<SearchResult *>(20);
    List<int> * docsToSearch = new List<int>(20);
    char * text_to_break = (char *)malloc(strlen(query)+1);
    strcpy(text_to_break, query);
    char * pch = strtok (text_to_break," \t");
    int query_words_counter = 0;
    while (pch != NULL){ //for each word in query Until 10
        query_words_counter++;
        if(query_words_counter>10)break;
        //printf("%s\n", pch);
        List<TrieDataNode *> * wordApp = this->find_word_appearance(pch);
        if(!wordApp){
            pch = strtok(NULL, " \t");
            continue;
        }
        for(int i = 0; i < wordApp->getLen(); ++i){
            int doc_id = wordApp->value(i)->doc_id;
            if(!docsToSearch->exists(doc_id))docsToSearch->appendElement(doc_id);
        }
        //go to next
        pch = strtok(NULL, " \t");
    }
    free(text_to_break);
    int NoOfTexts = texts->getLen();
    printf("going to search %d documents\n", docsToSearch->getLen());

    for (int j = 0; j < docsToSearch->getLen(); ++j) {
        int doc_id = docsToSearch->value(j);
        double score = this->docScore(query, texts->value(doc_id), doc_id, stringsWordsCounter->value(doc_id), AvgWords, NoOfTexts);
        results->appendElement(new SearchResult(doc_id, query, score));
    }

    quickSort(results, 0, results->getLen()-1);
    delete docsToSearch;
    return  results;
}

double Trie::docScore(char * query, char * doc, int doc_id, int wordsNo, double AvgWords, int NoOfDocs){
    double idf, result=0;
    int noOfDocsWithThisWord, frequencyInDoc=0;
    char * text_to_break = (char *)malloc(strlen(query)+1);
    strcpy(text_to_break, query);
    char * pch = strtok(text_to_break," \t");
    while (pch != NULL){ //for each word in query
        //printf("%s\n", pch);
        List<TrieDataNode *> * wordApp = this->find_word_appearance(pch);
        if(wordApp){
            for (int i = 0; i < wordApp->getLen(); ++i) {
                if(wordApp->value(i)->doc_id==doc_id)frequencyInDoc = wordApp->value(i)->times;
            }
        }
        noOfDocsWithThisWord = (wordApp)?(wordApp->getLen()):(0);
        //compute IDF
        idf = std::log((NoOfDocs-noOfDocsWithThisWord+0.5)/(noOfDocsWithThisWord+0.5));
        //add to sum
        result += idf * (frequencyInDoc*(1.2+1))/(frequencyInDoc+(1.2)*(1-0.75+0.75*(wordsNo/AvgWords)));
        //go to next
        frequencyInDoc=0;
        pch = strtok(NULL, " \t");
    }
    free(text_to_break);
    return result;
}

