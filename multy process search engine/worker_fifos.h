//
// Created by spyros on 10/4/2018.
//

#ifndef HW2_WORKER_FIFOS_H
#define HW2_WORKER_FIFOS_H

#define MAXBUFF 4000
#define FIFO "/tmp/syspro.hw2.fifo"
#define PERMS   0666


bool unlink_fifos(int pid){
    char fifo_name[80];
    sprintf(fifo_name, "%s.%d_master",FIFO, pid);
    if ( unlink(fifo_name) < 0) {
        perror("worker: can't unlink \n");
    }
    sprintf(fifo_name, "%s.%d_slave",FIFO, pid);
    if ( unlink(fifo_name) < 0) {
        perror("worker: can't unlink \n");
    }
    return true;
}

int open_for_write(int pid){
    char fifo_name[80];
    int writefd;
    sprintf(fifo_name, "%s.%d_slave",FIFO, pid);
    if ( (writefd = open(fifo_name, O_WRONLY))  < 0){
        perror("worker: can't open write fifo\n");
        exit(1);
    }
    return writefd;
}

int open_for_read(int pid){
    char fifo_name[80];
    int readfd;
    sprintf(fifo_name, "%s.%d_master",FIFO, pid);
    if ( (readfd = open(fifo_name, O_RDONLY | O_NONBLOCK))  < 0)  {
        perror("worker: can't open read fifo\n");
        exit(1);
    }
    return readfd;
}

bool write_to_pipe(int writefd, char * message){
    if (write(writefd, message, strlen(message)) == -1){
        perror("worker: filename write error");
    }

}
int read_from_pipe(int readfd, char * result){
    int n = read(readfd, result, MAXBUFF);
    if(n>=0)result[n] = '\0';
    return  n;
}
#endif //HW2_WORKER_FIFOS_H
