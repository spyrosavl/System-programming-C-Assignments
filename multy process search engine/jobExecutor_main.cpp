#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <signal.h>
#include <time.h>
#include "List.h"
#include "main.h"
#include "Trie.h"
#include "executor_fifos.h"
extern int errno;
void read_queries();
List<char *> *dir_paths;
List<pid_t > *workers_pids;
List<int> * writeFIFO;
List<int> * readFIFO;
int no_of_workers;
char * lineBuf = NULL;
int terminateNow=0;

void terminate(int signal){
    terminateNow=1;
    printf("Terminating...\n");
    int exit_status;
    int alive_children=no_of_workers;

    for (int k = 0; k < no_of_workers; ++k) {
        close(writeFIFO->value(k));
        close(readFIFO->value(k));
        unlink_fifos(k);
        kill(workers_pids->value(k),SIGTERM);
        wait(&exit_status);
        alive_children--;
        printf("Alive workers: %d\n", alive_children);
    }
    if(lineBuf)free(lineBuf);
    delete workers_pids;
    delete writeFIFO;
    delete readFIFO;
    delete dir_paths;

    exit(0);
}

int generate_worker(int i){

    List<char *> *path_for_worker = new List<char *>(10,1);

    char * proc_id_str = (char *)malloc(80);
    sprintf(proc_id_str, "%d", i);
    path_for_worker->appendElement(proc_id_str);

    //assign dirs to worker
    for (int j = 0; j < dir_paths->getLen(); ++j) {
        if(j%no_of_workers==i){
            path_for_worker->appendElement(dir_paths->value(j));
        }
    }
    path_for_worker->appendElement(NULL);
    pid_t worker_pid;

    if((worker_pid = fork())==0){ // from child
        execvp("./worker", path_for_worker->Array);
    }else{
        free(proc_id_str);
        delete path_for_worker;
        return worker_pid;
    }
}

void regenerate_child(int signal){
    if(terminateNow)return;
    pid_t pid;
    pid = wait(NULL);
    printf("Child killed! pid=%d\n", pid);

    for (int i = 0; i < no_of_workers; ++i) {
        if(workers_pids->value(i)==pid){
            workers_pids->update(i, generate_worker(i));
            printf("New worker generated with pid=%d\n", workers_pids->value(i));
            break;
        }
    }

    //read_queries();
}
void wc(List<char *> * Results){
    int chars=0, words=0, lines=0;
    int tmp_c, tmp_w, tmp_l;
    for (int i = 0; i < Results->getLen(); ++i) {
        sscanf(Results->value(i), "%d %d %d", &tmp_c, &tmp_w, &tmp_l);
        chars += tmp_c;
        words += tmp_w;
        lines += tmp_l;
    }
    printf("Characters: %d\nWords: %d\nLines: %d\n", chars, words, lines);
}
void minCount(List<char *> * Results){
    if(Results->getLen()==0){
        printf("No results!\n");
    }
    int min=99999, minIndex=0, value;
    char buffer[200];
    for (int i = 0; i < Results->getLen(); ++i) {
        sscanf(Results->value(i), "%s %d", &buffer, &value);
        if(strcmp("empty", buffer)==0)
            continue;
        if(value<min){
            min=value;
            minIndex=i;
        }
    }
    printf("%s\n", Results->value(minIndex));
}
void maxCount(List<char *> * Results){
    if(Results->getLen()==0){
        printf("No results!\n");
    }
    int max=0, maxIndex=0, value;
    for (int i = 0; i < Results->getLen(); ++i) {
        sscanf(Results->value(i), "%*s %d", &value);
        if(value>max){
            max=value;
            maxIndex=i;
        }
    }
    printf("%s\n", Results->value(maxIndex));
}

void read_queries(){
    printf("\nWaiting....\n");
    char buffer[10000];
    char command[20];
    int timeoutTime=0;
    lineBuf = (char *)malloc(10050);
    char result[4000];
    if(!lineBuf){
        perror("No memory for line buffer!");
    }
    while( fgets(lineBuf, 10050, stdin) ){
        sscanf(lineBuf, "%19s %[^\n]s", &command, &buffer);
        char * query = (char *)malloc(strlen(command)+strlen(buffer)+6);
        sprintf(query, "%s %s\n", command, buffer);

        if(strcmp(command, "/exit")!=0 && strcmp(command, "/wc")!=0 && strcmp(command, "/search")!=0 && strcmp(command, "/maxcount")!=0 && strcmp(command, "/mincount")!=0) {
            free(query);
            printf("Unknown command! Try again...\n");
            continue;
        }

        if(strcmp(command, "/exit")==0) {
            free(query);
            terminate(0);
        }

        if(strcmp(command, "/search")==0) {
            timeoutTime=0;
            sscanf(query, "%[^-] %*s %d", query, &timeoutTime);
            if(timeoutTime>0){
                printf("timeout: %d secs\n", timeoutTime);
            }
            if(timeoutTime<0)timeoutTime=0;
        }

        for (int k = 0; k < no_of_workers; ++k) {
            write_to_pipe(writeFIFO->value(k), query);
            kill(workers_pids->value(k),SIGUSR1);
        }

        List<int> * workersOnTime = new List<int>(no_of_workers);
        if(timeoutTime){
            sleep(timeoutTime);
        }

        int n;
        List<char *> * Results = new List<char *>(10);
        for (int i = 0; i < no_of_workers; ++i) {
            if(!result)perror("cant find more space!");

            do{
                usleep(1000*100);
            }while((n=read_from_pipe(readFIFO->value(i), result))<=0 && timeoutTime==0);

            if(n>0 && timeoutTime==0){
                char * result_tmp = (char *)malloc(4001); if(!result_tmp)perror("No space");
                strcpy(result_tmp, result);
                Results->appendElement(result_tmp);
            }
            //check for timeout
            workersOnTime->appendElement(i);
            while(read_from_pipe(readFIFO->value(i), result)>0){
                usleep(1000);
                char * result_tmp = (char *)malloc(4001); if(!result_tmp)perror("No space");
                strcpy(result_tmp, result);
                Results->appendElement(result_tmp);
            }
        }

        if(strcmp(command, "/wc")==0){
            wc(Results);
        }else if(strcmp(command, "/maxcount")==0){
            maxCount(Results);
        }else if(strcmp(command, "/mincount")==0){
            minCount(Results);
        }else if(strcmp(command, "/search")==0){
            for (int i = 0; i < Results->getLen(); ++i) {
                printf("%s\n", Results->value(i));
            }
        }
        delete Results;

        if(timeoutTime){
            printf("%d out of %d workers responded on %d seconds.\n", workersOnTime->getLen(), no_of_workers, timeoutTime);
        }

        for (int i = 0; i < no_of_workers; ++i) {
            if(workersOnTime->exists(i))
                continue;
            do{
                usleep(1000*100);
            }while(read_from_pipe(readFIFO->value(i), result)<=0);

            while(read_from_pipe(readFIFO->value(i), result)>0){
                //printf("%s\n", result);
                usleep(1);
            }
        }

        free(query);
        delete workersOnTime;
        printf("\nWaiting....\n");
    }
}


int main(int argc, char *argv[]) {

    static struct sigaction act, childTermAct;
    act.sa_handler=terminate;
    sigfillset(&(act.sa_mask));
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    childTermAct.sa_handler=regenerate_child;
    sigfillset(&(childTermAct.sa_mask));
    sigaction(SIGCHLD, &childTermAct, NULL);

    char *input_file=NULL;
    dir_paths = new List<char *>(10);
    workers_pids = new List<pid_t >(10);
    no_of_workers=-1;
    //read_parameters
    while ((argc > 1))
    {
        if(strcmp(argv[1],"-d")==0)
            input_file = argv[2];
        else if(strcmp(argv[1],"-w")==0)
            no_of_workers = atoi(argv[2]);
        ++argv;
        --argc;
    }
    if(!input_file){
        fprintf(stderr, "specify input file with -d\n");
        exit(1);
    }
    if(!no_of_workers || no_of_workers<=0){
        fprintf(stderr, "invalid w number. provide one with -w\n");
        exit(1);
    }

    //open file pointers
    FILE *inputFilePointer = fopen (input_file , "r");
    if(!inputFilePointer){
        fprintf(stderr, "error opening file %s\n", input_file);
        exit(1);
    }

    char * dirpath;
    int scan_return_code;
    while( (scan_return_code=fscanf(inputFilePointer, "%ms", &dirpath)) != EOF ){
        if(scan_return_code!=1){
            fprintf(stderr, "cannot read %d\n",scan_return_code);
            exit(1);
        }else{
            dir_paths->appendElement(dirpath);
        }
    }
    fclose(inputFilePointer);

    //create workers
    for (int i = 0; i < no_of_workers; ++i) {
        create_fifos(i);
        workers_pids->appendElement(generate_worker(i));
    }

    writeFIFO = new List<int>(no_of_workers);
    readFIFO = new List<int>(no_of_workers);

    //send messages to workers
    for (int k = 0; k < no_of_workers; ++k) {
        //printf("create write fifos: %d\n", k);
        writeFIFO->appendElement(open_for_write(k));
    }
    for (int l = 0; l < no_of_workers; ++l) {
        //printf("create read fifos: %d\n", l);
        readFIFO->appendElement(open_for_read(l));
    }

    while(1) {
        read_queries();
    }

    terminate(0);
}