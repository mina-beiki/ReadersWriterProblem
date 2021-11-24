#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

int pid, mainPid;
int shmid;

typedef struct {
    int ctr, readerCtr;
    pthread_mutex_t mutex;
} shNode; //nodes of shared memory which each is a struct

void readerFunc();
void writerFunc();

int main() {
    printf("Creating the shared memory:\n");

    if ((shmid = shmget(IPC_PRIVATE, sizeof(shNode), IPC_CREAT | 0666)) < 0) {
        perror("shmget error");
        exit(-1);
    }

    shNode* shMem; //shared memory
    if (( shMem = (shNode *)shmat(shmid, NULL, 0)) == (shNode *) -1) {
        perror("shmat error");
        exit(-1);
    }

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&(shMem->mutex), &attr);

    if (shmdt(shMem) == -1) {
        perror("shmdt error");
        exit(-1);
    }


    mainPid=getpid();
    //create writer process:
    pid = fork();
    if(pid ==0){
        writerFunc();
        return 0;
    }
    //create readers:
    for(int i=0 ; i<2 ; i++){
        int pidTemp = getpid();
        if(pidTemp == mainPid){
            pid = fork();
        }else{
            break;
        }
    }
    if(pid == 0){
        readerFunc();
        return 0;
    }
    if( getpid() == mainPid){
        for(int i=0 ; i<3 ; i++){
            wait(NULL); //wait on all readers and writers
        }
    }

    //deAttach shared memory:
    if(-1 == (shmctl(shmid, IPC_RMID, NULL)))
    {
        perror("shmctl error");
        exit(-1);
    }
    return 0;
}

void writerFunc(){
    shNode* shMem;
    if ((shMem = (shNode *)shmat(shmid, NULL, 0)) == (shNode *) -1) {
        perror("shmat error");
        exit(-1);
    }
    bool flag = true;
    while(flag){
        pthread_mutex_lock(&(shMem->mutex));
        shMem->ctr ++;
        printf("Writer has access.\n");
        printf("PID = %d\n",getpid());
        printf("Counter = %d\n",shMem->ctr);
        //check if it has reached the limit:
        if(shMem->ctr >= 5){
            flag = false;
        }
        pthread_mutex_unlock(&(shMem->mutex));
    }
    if (shmdt(shMem) == -1) {
        perror("shmdt error");
        exit(-1);
    }
    sleep(1);

}

void readerFunc(){
    shNode* shMem;
    if ((shMem = (shNode *)shmat(shmid, NULL, 0)) == (shNode *) -1) {
        perror("shmat error");
        exit(-1);
    }
    bool flag = true;
    while(flag){
        shMem->readerCtr ++;
        if(shMem->readerCtr == 1){ //first reader should lock the mutex
            pthread_mutex_lock(&(shMem->mutex));
        }
        printf("Reader has access.\n");
        printf("PID = %d\n",getpid());
        printf("Counter = %d\n",shMem->ctr);
        //check if it has reached the limit:
        if(shMem->ctr >= 5){
            flag = false;
        }

        shMem->readerCtr --;
        if(shMem->readerCtr == 0) {
            pthread_mutex_unlock(&(shMem->mutex));
        }

    }
    if (shmdt(shMem) == -1) {
        perror("shmdt error");
        exit(-1);
    }
}
