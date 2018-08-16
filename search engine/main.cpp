#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>
#include "List.h"
#include "main.h"
#include "Trie.h"


int main(int argc, char *argv[]) {
    //srand((unsigned int) time(NULL));
    //printf("%u\n", (unsigned int) time(NULL));

    const char *input_file=NULL;
    int scan_return_code;
    int max_num_of_results=-1;
    double AvgWords=0;

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    //read_parameters
    while ((argc > 1))
    {
        if(strcmp(argv[1],"-i")==0)
            input_file = argv[2];
        else if(strcmp(argv[1],"-k")==0)
            max_num_of_results = atoi(argv[2]);
        ++argv;
        --argc;
    }
    if(!input_file){
        fprintf(stderr, "specify input file with -i\n");
        exit(1);
    }
    if(!max_num_of_results || max_num_of_results<=0){
        fprintf(stderr, "invalid k number. provide one with -k\n");
        exit(1);
    }

    //open file pointers
    FILE *inputFilePointer = fopen (input_file , "r");
    if(!inputFilePointer){
        fprintf(stderr, "error opening file %s\n", input_file);
        exit(1);
    }

    List<char *> * stringMap = new List<char *>(20);
    List<int> * stringsWordsCounter = new List<int>(20);
    Trie * wordsTrie = new Trie();
    //read data
    int inputId;
    char * inputText;
    int counter = 0, cur_id=0;
    while( (scan_return_code=fscanf(inputFilePointer, "%d %m[^\n]", &inputId, &inputText)) != EOF ){
        if(scan_return_code!=2){
            fprintf(stderr, "cannot read\n");
            delete stringMap;
            delete stringsWordsCounter;
            delete  wordsTrie;
            exit(1);
        }else{
            if(cur_id != 0 && cur_id!=inputId){
                fprintf(stderr, "invalid ID number at line %d | ID = %d\n", counter, inputId);
                exit(1);
            }
            cur_id=inputId;
            if(counter==0){
                stringMap->start_from = inputId;
                stringsWordsCounter->start_from = inputId;
            }
            stringMap->appendElement(inputText);
            //add words to hashTable
            char * pch;
            int words_counter=0;
            char * text_to_break = (char *)malloc(strlen(inputText)+1);
            strcpy(text_to_break, inputText);
            pch = strtok (text_to_break," \t");
            while (pch != NULL){
                //from all texts
                //printf("%s\n", pch);
                wordsTrie->add_word(pch,inputId);
                words_counter++;
                //go to next
                pch = strtok(NULL, " \t");
            }
            free(text_to_break);
            AvgWords+=words_counter;
            stringsWordsCounter->appendElement(words_counter);
            counter++; cur_id++;
        }
    }
    int total_words = AvgWords;
    int console_width = w.ws_col;

    AvgWords = ((double)AvgWords)/stringMap->getLen();
    printf("No of lines = %d\nAvg Words %fl\n", stringMap->getLen(),AvgWords);
    printf("Console width: %d\n", console_width);
    printf("\nWaiting....\n");
    char command[20];
    char * args;
    while( (scan_return_code=scanf("%19s%m[^\n]", &command, &args)) != EOF ){
        if(strcmp(command, "/df")==0){
            char * word = NULL;
            int id=-1;
            if(args){
                sscanf(args, "%ms", &word);
            }
            if(word){
                printf("%20s %10d\n", word, wordsTrie->appearance_times(word, -1));
                free(word);
            }else{
                List<List<TrieDataNode * > *> * AllWordsFreq= wordsTrie->all_words_appearances();
                for (int i = 0; i < AllWordsFreq->getLen(); ++i) {
                    if(AllWordsFreq->value(i)->getLen()>0){
                        char * cur_word = AllWordsFreq->value(i)->value(0)->word;
                        int times_counter=0;
                        for (int j = 0; j < AllWordsFreq->value(i)->getLen(); ++j) {
                            times_counter+=AllWordsFreq->value(i)->value(j)->times;
                        }
                        printf("%20s %10d\n", cur_word, times_counter);
                    }
                }
                delete AllWordsFreq;
            }
        }else if(strcmp(command, "/tf")==0){
            char * word;
            int id=-1;
            if(!args){
                printf("Error in input! No query words!\n");
                printf("\nWaiting....\n");
                continue;
            }
            sscanf(args, "%d %ms", &id, &word);
            if(id>=0 && word){
                printf("%20s %10d\n", word, wordsTrie->appearance_times(word, id));
            }else{
                printf("Error in input");
            }
            free(word);

        }else if(strcmp(command, "/search")==0) {
            char *query;
            if(!args){
                printf("Error in input! No query words!\n");
                printf("\nWaiting....\n");
                continue;
            }
            sscanf(args, "%m[^\n]}", &query);
            if (!query) {
                printf("Error in input\n");
                continue;
            }

            List<SearchResult *> * results;
            List<char *> * QueryWordsList = new List<char *>(10,1); //dont delete items inside on destroy
            char * pointer_line = (char*)malloc(console_width+1);

            //add query words to list
            char * text_to_break = (char *)malloc(strlen(query)+1);
            strcpy(text_to_break, query);
            char * pch = strtok (text_to_break," \t");
            while (pch != NULL){
                QueryWordsList->appendElement(pch);
                //go to next
                pch = strtok(NULL, " \t");
            }

            //search
            results = wordsTrie->search(query, stringMap, stringsWordsCounter, AvgWords);

            //print results
            for (int i = 0; i < ((results->getLen()<max_num_of_results)?(results->getLen()):(max_num_of_results)); ++i) {
                printf("%5d. (%5d) [%5f] ", i, results->value(i)->document_id, results->value(i)->score);
                int cur_width = 6+1+5+1+5+5+5;
                int pointer_line_pos=cur_width-2;
                char * text_to_print = stringMap->value(results->value(i)->document_id);

                char * text_to_break = (char *)malloc(strlen(text_to_print)+1);
                strcpy(text_to_break, text_to_print);
                char * pch = strtok (text_to_break," \t");

                for (int j = 0; j < console_width; ++j) {
                    pointer_line[j] = ' ';
                }

                while (pch != NULL){
                    //from all texts
                    //printf("%s\n", pch);
                    if(cur_width+strlen(pch)+1<=console_width){ //check if you can put one more word
                        printf("%s ", pch); //print word
                        cur_width += strlen(pch)+1;
                        int word_found_in_query=0;
                        for (int j = 0; j < QueryWordsList->getLen(); ++j){ //search if word in query
                            if(strcmp(pch,QueryWordsList->value(j))==0){
                                word_found_in_query=1;
                                break;
                            }
                        }
                        for (int k = 0; k < strlen(pch); ++k) {
                            pointer_line[pointer_line_pos] = (word_found_in_query)?('^'):(' ');
                            pointer_line_pos += 1;
                        }
                        pointer_line_pos++;
                        pointer_line[pointer_line_pos+1] = ' ';
                    }else{
                        pointer_line[pointer_line_pos+1] = '\0';
                        printf("\n%s\n", pointer_line);
                        cur_width = 0;
                        pointer_line_pos=0;
                        for (int j = 0; j < console_width; ++j) {
                            pointer_line[j] = ' ';
                        }
                        continue;
                    }
                    //go to next
                    pch = strtok(NULL, " \t");
                }

                pointer_line[console_width] = '\0';
                printf("\n%s\n", pointer_line);

                free(text_to_break);
            }
            free(pointer_line);
            free(query);
            free(text_to_break);
            delete results;
            delete QueryWordsList;
        }else if(strcmp(command, "/exit")==0){
            break;
        }

        if(args)free(args);
        printf("\nWaiting....\n");
    }

    fclose(inputFilePointer);
    delete wordsTrie;
    delete stringMap;
    delete stringsWordsCounter;
    return 0;
}