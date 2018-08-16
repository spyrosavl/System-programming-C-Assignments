#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include "List.h"
#include "main.h"
#include "Trie.h"
#include "worker_fifos.h"

int proc_id, readfd, writefd;
List<char *> *file_paths;
List<char *> *lines;
Trie * files_trie;
int total_chars = 0, total_words =0, total_lines = 0;
FILE *logsFile;

void log_worker(char * command, char * query, char * paths){
    time_t mytime = time(NULL);
    char * time_str = ctime(&mytime);
    time_str[strlen(time_str)-1] = '\0';
    fprintf(logsFile, "%s: %s: %s : %s\n", time_str, command, query, paths);
}

void search(char * query){
    //int string_len=0;
    //long pipe_size=fpathconf(writefd, _PC_PIPE_BUF);
    List<SearchResult *> * Results = files_trie->search(query);
    List<int> * uniqueFileIDs = new List<int>(10);
    char * paths = (char *)malloc(Results->getLen()*80);
    char * result_string = (char *)malloc(4000);

    if(!result_string || !paths){
        perror("failed to malloc for search results");
    }
    result_string[0] = '\0';
    paths[0] = '\0';

    //sleep(rand() % 20);for debugging

    char * line3900chars = (char *)malloc(3901);
    char buffer[200];
    for (int i = 0; i < Results->getLen(); ++i) {
        SearchResult * current = Results->value(i);

        char * line = lines->value(current->line_id);
        int j, line_len = (int)strlen(line);
        //limit line size to 3900
        for (j = 0; j < 3900 && j < line_len; ++j) { line3900chars[j] = line[j]; } line[j] = '\0';

        sprintf(result_string,"%s %d %s\n",file_paths->value(current->file_id) ,current->line_id, lines->value(current->line_id));
        if(!uniqueFileIDs->exists(current->file_id)){
            sprintf(paths, "%s%s, ", paths, file_paths->value(current->file_id));
            uniqueFileIDs->appendElement(current->file_id);
        }
        write_to_pipe(writefd, result_string);
    }
    if(Results->getLen() == 0){
        sprintf(result_string,"empty\n");
        write_to_pipe(writefd, result_string);
    }
    log_worker("search", query, paths);
    free(result_string);
    free(paths);
    free(line3900chars);
    delete uniqueFileIDs;
    delete Results;
}

void maxcount(char * keyword){
    List<SearchResult *> * Results = files_trie->search(keyword);
    List<int> * fileCounter = new List<int>(file_paths->getLen());
    int maxIndex = 0, max = 0;

    for (int k = 0; k < file_paths->getLen(); ++k) {
        fileCounter->appendElement(0);
    }

    for (int j = 0; j < Results->getLen(); ++j) {
        int file_index = Results->value(j)->file_id;
        fileCounter->update(file_index, fileCounter->value(file_index) + Results->value(j)->times);
    }

    for (int i = 0; i < fileCounter->getLen(); ++i) {
        if(fileCounter->value(i) > max){
            max = fileCounter->value(i);
            maxIndex = i;
        }
    }

    char * result_string = (char *)malloc(200);
    if(Results->getLen()>0){
        sprintf(result_string,"%s %d\n",file_paths->value(maxIndex), max);
    }else{
        sprintf(result_string,"%s %d\n", "empty", 0);
    }
    log_worker("maxcount", keyword, "");
    write_to_pipe(writefd, result_string);
    free(result_string);
    delete Results;
    delete fileCounter;
}

void mincount(char * keyword){
    List<SearchResult *> * Results = files_trie->search(keyword);
    List<int> * fileCounter = new List<int>(file_paths->getLen());
    int minIndex = 0, min = 999999;

    for (int k = 0; k < file_paths->getLen(); ++k) {
        fileCounter->appendElement(0);
    }

    for (int j = 0; j < Results->getLen(); ++j) {
        int file_index = Results->value(j)->file_id;
        fileCounter->update(file_index, fileCounter->value(file_index) + Results->value(j)->times);
    }

    for (int i = 0; i < fileCounter->getLen(); ++i) {
        if(fileCounter->value(i) < min){
            min = fileCounter->value(i);
            minIndex = i;
        }
    }

    char * result_string = (char *)malloc(200);
    if(Results->getLen()>0){
        sprintf(result_string,"%s %d\n",file_paths->value(minIndex), min);
    }else{
        sprintf(result_string,"%s %d\n", "empty", 0);
    }
    log_worker("mincount", keyword, "");
    write_to_pipe(writefd, result_string);
    free(result_string);
    delete Results;
    delete fileCounter;
}

void wc(){
    char * result_string = (char *)malloc(100);
    sprintf(result_string,"%d %d %d\n",total_chars, total_words, total_lines);
    log_worker("wc", "", "");
    write_to_pipe(writefd, result_string);
    free(result_string);
}

void signal_handler(int a){
    char buffer[MAXBUFF];
    read_from_pipe(readfd, buffer);
    //printf("Command is: %s\n", buffer);
    char command[20];
    char * args, * query;
    if( (sscanf(buffer, "%19s%m[^\n]", &command, &args)) != EOF ) {
        if (strcmp(command, "/search") == 0) {

            if (!args) {
                printf("Error in input! No query words!\n");
                printf("\nWaiting....\n");
                exit(1);
            }
            sscanf(args, "%m[^\n]", &query);
            if (!query) {
                printf("Error in input\n");
                exit(1);
            }

            search(query);
            free(query);
        }else if(strcmp(command, "/maxcount") == 0){
            sscanf(args, "%ms", &query);
            maxcount(query);
            free(query);

        }else if(strcmp(command, "/mincount") == 0){
            sscanf(args, "%ms", &query);
            mincount(query);
            free(query);

        }else if(strcmp(command, "/wc") == 0){
            wc();
        }
    }
}

void terminate(int signal){
    close(writefd);
    close(readfd);
    fclose(logsFile);
    delete files_trie;
    delete file_paths;
    delete lines;
    exit(0);
}

int main(int argc, char *argv[]) {

    static struct sigaction terminate_act, message_act;

    terminate_act.sa_handler=terminate;
    sigfillset(&(terminate_act.sa_mask));
    sigaction(SIGINT, &terminate_act, NULL);
    sigaction(SIGTERM, &terminate_act, NULL);

    message_act.sa_handler=signal_handler;
    sigfillset(&(message_act.sa_mask));
    sigaction(SIGUSR1, &message_act, NULL);

    file_paths = new List<char *>(10);
    lines = new List<char *>(20);
    files_trie = new Trie();

    const char *input_file=NULL;

    char logPath[50];
    sprintf(logPath, "log/Worker_%d.txt", (int)getpid());
    logsFile = fopen(logPath, "w");

    char proc_id_str[80];
    strcpy(proc_id_str, argv[0]);
    proc_id = atoi(proc_id_str);
    //read_parameters
    while ((argc > 1))
    {
        DIR *dp;
        struct dirent *ep;
        dp = opendir(argv[argc-1]);
        if (dp != NULL)        {
            while (ep = readdir (dp)){
                if(ep->d_type != DT_DIR){
                    char * copy = (char *)malloc(strlen(argv[argc-1]) + strlen(ep->d_name) + 2);
                    sprintf(copy, "%s/%s", argv[argc-1], ep->d_name);
                    file_paths->appendElement(copy);
                }
            }
            (void) closedir (dp);
        }else{
            perror ("Couldn't open the directory");
        }
        //printf("Worker %d %s\n", proc_id, argv[argc-1]);
        --argc;
    }
    if(file_paths->getLen()==0){
        printf("no dir given to worker %d\n", proc_id);
    }

    for (int i = 0; i < file_paths->getLen(); ++i) {
        //for each file
        char *buffer = NULL;
        size_t buffsize = 0;
        size_t characters;

        //printf("file %d %s\n", i, file_paths->value(i));
        FILE *inputFilePointer = fopen(file_paths->value(i), "r");
        if(!inputFilePointer){
            fprintf(stderr, "error opening file %s\n", file_paths->value(i));
            exit(1);
        }
        while( (characters = getline(&buffer,&buffsize,inputFilePointer)) != -1 ){
            char * copy = (char *)malloc(characters + 1);
            strcpy(copy, buffer);
            total_chars += strlen(copy);
            lines->appendElement(copy);
            //add words to trie
            char * text_to_break = (char *)malloc(strlen(buffer)+1);
            strcpy(text_to_break, buffer);
            char * pch = strtok (text_to_break," \t\n");
            while (pch != NULL){
                total_words++;
                files_trie->add_word(pch,i,total_lines);
                //go to next word
                pch = strtok(NULL, " \t\n");
            }
            free(text_to_break);
            total_lines++;
        }

        free(buffer);
        fclose(inputFilePointer);
    }

    readfd = open_for_read(proc_id);
    writefd = open_for_write(proc_id);

    while(1){
        sleep(1);
    }


}