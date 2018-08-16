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
List<int> * Buffer;
List<pthread_t *> * threads;
char * root_dir=NULL;
int port=-1, sock, command_sock, newsock, command_port=-1, thread_count=-1;
std::chrono::steady_clock::time_point start; long pages=0, bytes=0;

void perror_exit(const char *message);
void * server_thread(void * ptr);
void terminate(int status);
void place(List<int> * buffer, int data);
int  obtain(List<int> * buffer);
void * command_server_thread(void * ptr);
double timediff(clock_t t1, clock_t t2);

main(int argc, char *argv[]) {
    start = std::chrono::steady_clock::now();
    signal(SIGINT, terminate);
    struct sockaddr_in server, client;
    socklen_t clientlen;
    struct sockaddr *serverptr=(struct sockaddr *)&server;
    struct sockaddr *clientptr=(struct sockaddr *)&client;

    pthread_mutex_init(&mtx, 0);
    pthread_cond_init(&cond_nonempty, 0);

    while ((argc > 1))
    {
        if(strcmp(argv[1],"-p")==0)
            port = atoi(argv[2]);
        else if(strcmp(argv[1],"-c")==0)
            command_port = atoi(argv[2]);
        else if(strcmp(argv[1],"-t")==0)
            thread_count = atoi(argv[2]);
        else if(strcmp(argv[1],"-d")==0)
            root_dir = argv[2];
        ++argv;
        --argc;
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
    if(!root_dir){
        perror_exit("provide root path with -d\n");
    }

    Buffer = new List<int>(20);
    threads = new List<pthread_t *>(thread_count);
    if(threads == NULL)
        perror_exit("Unable to malloc for threads!");

    /* Create sockets */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");
    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror_exit("setsockopt(SO_REUSEADDR) failed");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    /* Bind socket to address */
    if (bind(sock, serverptr, sizeof(server)) < 0)
        perror_exit("bind");
    /* Listen for connections */
    if (listen(sock, 5) < 0)
        perror_exit("listen");


    //create command server thread//
    pthread_t * thread = (pthread_t *)malloc(sizeof(pthread_t));
    if(thread == NULL)
        perror_exit("Unable to malloc for thread!");
    threads->appendElement(thread);
    pthread_create(thread, NULL, command_server_thread, NULL);
    /****************************/

    printf("Listening for connections on port %d\n", port);
    for (int i = 0; i < thread_count; ++i) {
        pthread_t * thread = (pthread_t *)malloc(sizeof(pthread_t));
        if(thread == NULL)
            perror_exit("Unable to malloc for thread!");
        threads->appendElement(thread);
        pthread_create(thread, NULL, server_thread, NULL);
    }

    while (1) {
        clientlen = sizeof(client);
        /* accept connection from port*/
    	if ((newsock = accept(sock, clientptr, &clientlen)) < 0) 
            perror_exit("accept");
        place(Buffer, newsock);
        pthread_cond_signal(&cond_nonempty);
        printf("Accepted connection\n");
    }
}

void * server_thread(void * ptr){
    printf("Thread created!\n");
    char request[2000], response[200000], tmp_path[1000], path[1000], bufferLine[10000];
    FILE *htmlFile;
    time_t rawtime;

    while(1){
        int socket = obtain(Buffer);
        time (&rawtime);
        read(socket, request, 1999);
        sscanf(request, "%*s %s %*s\n", &tmp_path);
        sprintf(path,"%s%s",root_dir, tmp_path);
        printf("GET %s\n", path);

        if(access(path, F_OK) != -1){
            htmlFile = fopen(path, "r");
            struct stat st;
            stat(path, &st);
            unsigned int size = st.st_size;

            sprintf(response, "HTTP/1.1 200 OK\n");
            sprintf(response, "%sDate: %s", response, std::ctime(&rawtime));
            sprintf(response, "%sServer: myhttpd/1.0.0\n", response);
            sprintf(response, "%sContent-Length: %d\n", response, size);
            sprintf(response, "%sContent-Type: text/html\n", response);
            sprintf(response, "%sConnection: Closed\n\n", response);
            while (fgets(bufferLine, sizeof(bufferLine), htmlFile))
                sprintf(response, "%s%s", response, bufferLine);
            fclose(htmlFile);

            pages++;
            bytes+=size;
        }else if(errno == EACCES){
            sprintf(response, "HTTP/1.1 403 Forbidden\n");
            sprintf(response, "%sDate: %s", response, std::ctime(&rawtime));
            sprintf(response, "%sServer: myhttpd/1.0.0\n", response);
            sprintf(response, "%sContent-Length: 200\n", response);
            sprintf(response, "%sContent-Type: text/html\n", response);
            sprintf(response, "%sConnection: Closed\n\n", response);
            sprintf(response, "%s<html><h1>You are not allowed to access that file</h1></html>", response);
        }
        else{
            sprintf(response, "HTTP/1.1 404 Not Found\n");
            sprintf(response, "%sDate: %s", response, std::ctime(&rawtime));
            sprintf(response, "%sServer: myhttpd/1.0.0\n", response);
            sprintf(response, "%sContent-Length: 200\n", response);
            sprintf(response, "%sContent-Type: text/html\n", response);
            sprintf(response, "%sConnection: Closed\n\n", response);
            sprintf(response, "%s<html><h1>File Not Found</h1></html>", response);
        }

        if (write(socket, response, strlen(response)+1) < 0)
                perror_exit("write");

        close(socket);	  /* Close socket */
    }
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
            sprintf(response,"Server up for %.0f seconds, served %lu pages, %lu bytes\n", elapsed, pages, bytes);
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

void perror_exit(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void place(List<int> * buffer, int data) {
        pthread_mutex_lock(&mtx);
        buffer->appendElement(data);
        pthread_mutex_unlock(&mtx);
}

int obtain(List<int> * buffer) {
        int data = 0;
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
    printf(">> Terminate server \n");
    for (int i = 0; i < threads->getLen(); ++i) {
        pthread_cancel(*(threads->value(i)));
    }
    pthread_cond_destroy(&cond_nonempty);
    pthread_mutex_destroy(&mtx);
    delete threads;
    delete Buffer;
    exit(0);
}

double timediff(clock_t t1, clock_t t2) {
    double elapsed;
    elapsed = ((double)t2 - t1) / CLOCKS_PER_SEC * 1000;
    return elapsed;
}