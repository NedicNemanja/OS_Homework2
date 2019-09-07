#include "ErrorCodes.h"
#include "SharedMem.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>  //perror
//shared mem
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h> //memcpy
#include <sys/sem.h>  //semaphores
#include <unistd.h>
#include <time.h> //usleep

/*Get shared memory segment*/
int QueueInit(key_t shkey, size_t memSize) {
    //get keys
    //printf("Getting semaphore keys for SharedMemoryQueue %d:\n", (int)shkey);
    key_t sem_key;
    sem_key = ftok("./include/SharedMem.h", (int) shkey);
    if (sem_key == -1) {
        perror("ftok failed when getting semkeys.\n");
        exit(FTOK);
    }
    //get memory segment
    int shmid;
    if ((shmid = shmget(shkey, sizeof(SHMemQueue) + memSize, IPC_CREAT | 0600)) < 0) {
        perror("shmget: ");
        exit(SHMGET);
    }
    //initialize queue data
    char *mem = QueueAttach(shmid);
    struct SHMemQueue SHMqueue = {0, 0, sem_key};
    //write queue data to shared memory
    memcpy(mem, &SHMqueue, sizeof(SHMemQueue));
    QueueDetach(mem);
    return shmid;
}

/*Attach shared memory segment*/
char *QueueAttach(int shmid) {
    char *shmemory = NULL;
    if ((shmemory = shmat(shmid, NULL, 0)) == (char *) -1) {
        perror("shmat: ");
        exit(SHMAT);
    }
    return shmemory;
}

void QueueDeleteSemaphores(SHMemQueue *queue) {
    SemDelete(SemGet(queue->semkey));
}

void QueueDetach(char *shm) {
    shmdt(shm);
}

void QueueDelete(int shmid) {
    //delete shared memory
    shmctl(shmid, IPC_RMID, 0);
}

/*Insert Component in shared memory*/
void QueueInsert(SHMemQueue *queue, Component *comp) {
    char *dest = GetOffset(queue, queue->back);
    //move back
    memcpy(dest, comp, sizeof(Component));
    (queue->back)++;
}

/*Paint the next component of the queue*/
Component *QueuePaint(SHMemQueue *queue, int type) {
    //get component
    Component *comp = (Component *) GetOffset(queue, queue->next);
    (queue->next)++;
    //paint component (time based on type)
    usleep(paint_time[type - 1] * 1000000);
    return comp;
}

/*Check the next painted component of the queue*/
void QueueCheck(SHMemQueue *queue, int type) {
    //check component
    //printf("Checking type%d\n",type);
    (queue->painted)++;
    usleep(check_time[type - 1] * 1000000);
}

/*Assemble a component of each type into a new one*/
Product QueueAssemble(SHMemQueue **queues) {
    //get all 3 components
    Component *comps[3];
    for (int i = 0; i < 3; i++) {
        comps[i] = (Component *) GetOffset(queues[i], queues[i]->checked);
        (queues[i]->checked)++;
    }
    //assemble
    usleep(assemblage_time * 1000000);
    return CreateProduct(comps);
}

/*return pointer to element at offset*/
char *GetOffset(SHMemQueue *queue, int offset) {
    return (char *) queue + sizeof(SHMemQueue) + (offset * sizeof(Component));
}


/****************SEMAPHORE OPS************************************************/
key_t SemCreate(int shkey) {
    key_t sem_key = ftok("./include/SharedMem.h", (int) shkey);
    if (sem_key == -1) {
        perror("ftok failed when getting semkeys.\n");
        exit(FTOK);
    }
    return sem_key
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

