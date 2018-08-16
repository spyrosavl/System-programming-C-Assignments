/*inet_str_server.c: Internet stream sockets server */
#include <stdio.h>
#include <sys/wait.h>	     /* sockets */
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>	     /* internet sockets */
#include <netdb.h>	         /* gethostbyaddr */
#include <unistd.h>	         /* fork */
#include <stdlib.h>	         /* exit */
#include <ctype.h>	         /* toupper */
#include <signal.h>          /* signal */
#include <cstring>
#include <ctime>
#include <pthread.h>
#include <chrono>  // for high_resolution_clock
#include <sys/stat.h>
#include <errno.h>
#include "List.h"

using namespace std;

pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
List<char *> * UrlBuffer;
List<char *> * UrlVisited;
List<pthread_t *> * threads;
int port=-1, command_sock, newsock, command_port=-1, thread_count=-1;
char * host, * save_dir, * starting_url;
std::chrono::steady_clock::time_point start; long pages=0, bytes=0;

void perror_exit(const char *message);
void * crawler_thread(void * ptr);
void terminate(int status);
void place(List<char *> * buffer, char * data);
char *  obtain(List<char *> * buffer);
void * command_server_thread(void * ptr);
double timediff(clock_t t1, clock_t t2);

main(int argc, char *argv[]) {
    start = std::chrono::steady_clock::now();
    signal(SIGINT, terminate);

    pthread_mutex_init(&mtx, 0);
    pthread_cond_init(&cond_nonempty, 0);

    while ((argc > 1))
    {
        if(strcmp(argv[1],"-h")==0)
            host = argv[2];
        else if(strcmp(argv[1],"-p")==0)
            port = atoi(argv[2]);
        else if(strcmp(argv[1],"-c")==0)
            command_port = atoi(argv[2]);
        else if(strcmp(argv[1],"-t")==0)
            thread_count = atoi(argv[2]);
        else if(strcmp(argv[1],"-d")==0)
            save_dir = argv[2];

        if(argc==2){
            starting_url = argv[1];
        }
        ++argv;
        --argc;
    }

    if(!host){
        perror_exit("provide host with -h\n");
    }
    if(!port || port<=0){
        perror_exit("invalid port number. provide one with -p\n");
    }
    if(!command_port || command_port<=0){
        perror_exit("invalid commands port number. provide one with -c\n");
    }
    if(!thread_count || thread_count<=0){
        perror_exit("invalid thread count. provide with -t\n");
    }
    if(!save_dir){
        perror_exit("provide save dir with -d\n");
    }

    UrlBuffer = new List<char *>(20,1);
    UrlVisited = new List<char *>(20);
    threads = new List<pthread_t *>(thread_count);
    if(threads == NULL)
        perror_exit("Unable to malloc for threads!");


    //printf("Connecting to %s port %d\n", host, port);
    printf("Starting URL: %s\n", starting_url);


    //create command server thread//
    pthread_t * thread = (pthread_t *)malloc(sizeof(pthread_t));
    if(thread == NULL)
        perror_exit("Unable to malloc for thread!");
    threads->appendElement(thread);
    pthread_create(thread, NULL, command_server_thread, NULL);
    /****************************/

    for (int i = 0; i < thread_count; ++i) {
        pthread_t * thread = (pthread_t *)malloc(sizeof(pthread_t));
        if(thread == NULL)
            perror_exit("Unable to malloc for thread!");
        threads->appendElement(thread);
        pthread_create(thread, NULL, crawler_thread, NULL);
    }

    char * url_to_place = (char*)malloc(strlen(starting_url)+1);
    strcpy(url_to_place,starting_url);
    place(UrlBuffer, url_to_place);
    pthread_cond_signal(&cond_nonempty);

    for (int j = 0; j < threads->getLen(); ++j) {
        pthread_join(*threads->value(j), NULL);
    }
}

void * crawler_thread(void * ptr){
    //printf("Thread created!\n");
    int sock;
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
    struct hostent *rem;
    size_t size=9999;
    char response[200000];
    char request[1000];
    char line[size];
    char * response_ptr = NULL;

    while(1){
        response_ptr=response;
        char * url = obtain(UrlBuffer);

        /* Create socket */
        if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
            perror_exit("socket");
        /* Find server address */
        if ((rem = gethostbyname(host)) == NULL) {
            perror("gethostbyname"); exit(1);
        }
        server.sin_family = AF_INET;       /* Internet domain */
        memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
        server.sin_port = htons(port);         /* Server port */
        /* Initiate connection */
        if (connect(sock, serverptr, sizeof(server)) < 0)
            perror_exit("connect");

        printf("visiting url: %s\n", url);
        pages+=1;

        sprintf(request, "GET %s HTTP/1.1\n\n", url);
        if (write(sock, request, strlen(request)+1) < 0)
            perror_exit("write");

        if (read(sock, response_ptr, 199999) < 0)
            perror_exit("read");

        bytes+=strlen(response_ptr);

        sscanf(response_ptr, "%[^\n]s", line);response_ptr+=strlen(line)+1;
        sscanf(response_ptr, "%[^\n]s", line);response_ptr+=strlen(line)+1;
        sscanf(response_ptr, "%[^\n]s", line);response_ptr+=strlen(line)+1;
        sscanf(response_ptr, "%[^\n]s", line);response_ptr+=strlen(line)+1;
        sscanf(response_ptr, "%[^\n]s", line);response_ptr+=strlen(line)+1;
        sscanf(response_ptr, "%[^\n]s", line);response_ptr+=strlen(line)+1;
        sscanf(response_ptr, "%[^\n]s", line);response_ptr+=strlen(line)+1;

        //write to file
        char file_path[600],site_folder[200], folder_path[300];
        sprintf(file_path, "%s%s", save_dir, url);
        sscanf(url,"/%[^/]s",site_folder);
        sprintf(folder_path, "%s/%s", save_dir, site_folder);

        struct stat st = {0};
        if (stat(folder_path, &st) == -1) {
            mkdir(folder_path, 0700);
        }

        FILE *f = fopen(file_path, "ab+");
        if (f == NULL) {
            printf("Error opening file!\n");
            exit(1);
        }
        fprintf(f, "%s", response_ptr-2);
        fclose(f);


        sscanf(response_ptr, "%[^\n]s", line);response_ptr+=strlen(line)+1;
        sscanf(response_ptr, "%[^\n]s", line);response_ptr+=strlen(line)+1;

        //find links
        while(sscanf(response_ptr, "\t\t<a href=\"%[^\"]s", line) == 1){
            //printf("%s\n", line);
            char * url_to_place = (char*)malloc(strlen(line)+1);
            strcpy(url_to_place,line);
            place(UrlBuffer, url_to_place);

            response_ptr+=strlen(line)+1;
            sscanf(response_ptr, "%[^\n]s", line);response_ptr+=strlen(line)+1;
        }

        pthread_cond_signal(&cond_nonempty);
        close(sock);
    }
}

void perror_exit(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void place(List<char *> * buffer, char * data) {
    pthread_mutex_lock(&mtx);
    if(!UrlVisited->exists(data)){
        buffer->appendElement(data);
        UrlVisited->appendElement(data);
    }else{
        //printf("url %s EXISTS!\n", data);
        free(data);
    }
    pthread_mutex_unlock(&mtx);
}

char * obtain(List<char *> * buffer) {
    char * data = NULL;
    pthread_mutex_lock(&mtx);
    while (buffer->getLen() == 0) {
        //printf(">> Found Buffer Empty \n");
        pthread_cond_wait(&cond_nonempty, &mtx);
    }
    data = buffer->remove();
    pthread_mutex_unlock(&mtx);
    return data;
}

void terminate(int status){
    printf(">> Terminate crawler \n");
    for (int i = 0; i < threads->getLen(); ++i) {
        pthread_cancel(*(threads->value(i)));
    }
    //pthread_cond_destroy(&cond_nonempty);
    pthread_mutex_destroy(&mtx);
    delete threads;
    delete UrlBuffer;
    delete UrlVisited;
    exit(0);
}

double timediff(clock_t t1, clock_t t2) {
    double elapsed;
    elapsed = ((double)t2 - t1) / CLOCKS_PER_SEC * 1000;
    return elapsed;
}


void * command_server_thread(void * ptr){
    struct sockaddr_in command_server, client;
    socklen_t clientlen;
    struct sockaddr *command_serverptr=(struct sockaddr *)&command_server;
    struct sockaddr *clientptr=(struct sockaddr *)&client;

    if ((command_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");
    int enable=1;
    if (setsockopt(command_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror_exit("setsockopt(SO_REUSEADDR) failed");

    command_server.sin_family = AF_INET;
    command_server.sin_addr.s_addr = htonl(INADDR_ANY);
    command_server.sin_port = htons(command_port);

    if (bind(command_sock, command_serverptr, sizeof(command_server)) < 0)
        perror_exit("bind");
    if (listen(command_sock, 5) < 0)
        perror_exit("listen");

    printf("Listening for commands on port %d\n", command_port);

    char request[200],response[1000];
    int terminate_now=0;
    while (1) {
        clientlen = sizeof(client);
        /* accept connection from port*/
        if ((newsock = accept(command_sock, clientptr, &clientlen)) < 0)
            perror_exit("accept");

        read(newsock, request, 199);
        sscanf(request,"%s",request);
        printf("Accepted command connection %s.\n", request);

        if(strcmp(request,"STATS")==0){
            std::chrono::steady_clock::time_point end= std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000000;
            sprintf(response,"Server up for %.0f seconds, downloaded %lu pages, %lu bytes\n", elapsed, pages, bytes);
        }else if(strcmp(request,"SHUTDOWN")==0){
            sprintf(response,"OK\n");
            terminate_now=1;
        }else{
            sprintf(response,"WRONG COMMAND\n");
        }

        if (write(newsock, response, strlen(response)) < 0)
            perror_exit("write");

        close(newsock);
        if(terminate_now){
            terminate(0);
        }
    }
}