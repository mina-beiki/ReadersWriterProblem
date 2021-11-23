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

int shmid;

typedef struct {
    int ctr, read_ctr;
    pthread_mutex_t mutex;
    pthread_mutex_t rw_mutex;
} shNode; //nodes of shared memory which each is a struct


int main() {
    printf("creating the shared memory:\n");
    key_t key;
    key = 3232;

    if ((shmid = shmget(IPC_PRIVATE, sizeof(shNode), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(-1);
    }

    char *shm;
    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat");
        exit(-1);
    }
    return 0;
}
