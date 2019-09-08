#include "ErrorCodes.h"
#include "SharedMem.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h> //memcpy
#include <sys/sem.h>  //semaphores

/*Attach shared memory segment*/
char *QueueAttach(int shmid) {
    char *shmemory = NULL;
    if ((shmemory = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat: ");
        exit(SHMAT);
    }
    return shmemory;
}

void QueueDetach(char *shm) {
    shmdt(shm);
}

void QueueDelete(int shmid) {
    //delete shared memory
    shmctl(shmid, IPC_RMID, 0);
}

/****************SEMAPHORE OPS************************************************/
key_t SemCreate(int shkey) {
    key_t sem_key = ftok("./include/SharedMem.h", (int) shkey);
    if (sem_key == -1) {
        perror("ftok failed when getting semkeys.\n");
        exit(FTOK);
    }
    return sem_key;
}

int SemGet(key_t semkey) {
    int semid;
    //create semaphore
    if ((semid = semget(semkey, 1, IPC_CREAT | 0600)) < 0) {
        perror("semget: ");
        exit(SEMGET);
    }
    return semid;
}

void SemDown(int semid) {
    //decrease semaphore by 1
    struct sembuf sops;
    sops.sem_num = 0;
    sops.sem_op = -1;
    sops.sem_flg = 0;
    if (semop(semid, &sops, 1) != 0) {
        perror("SemDown failed.");
        exit(SEMOP);
    }
}

void SemUp(int semid) {
    //increase semaphore
    struct sembuf sops;
    sops.sem_num = 0;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    if (semop(semid, &sops, 1) != 0) {
        perror("SemUp failed.");
        exit(SEMOP);
    }
}

void SemDelete(int semid) {
    //delete semaphore
    //printf("value of sem on delete: %d\n", semctl(semid,0,GETVAL));
    if (semctl(semid, 0, IPC_RMID) == -1) { //delete
        perror("Failed to delete semaphore.");
        exit(SEMCTL);
    }
}

/**
 * Increase/Decrease semval of semid by value
 * @param semid semaphore id
 * @param value int to be added/subtracted to semval
 */
void SemOp(int semid, int value) {
    //increase semaphore
    struct sembuf sops;
    sops.sem_num = 0;
    sops.sem_op = value;
    sops.sem_flg = 0;
    if (semop(semid, &sops, 1) != 0) {
        perror("SemOp failed.");
        exit(SEMOP);
    }
}

