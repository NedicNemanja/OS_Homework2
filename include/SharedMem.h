#ifndef SHAREDMEM_H
#define SHAREDMEM_H

#include <sys/types.h>

/***********Shared Memory Ops*************************************/
char* QueueAttach(int shmid);
void QueueDetach(char* shm);
void QueueDelete(int shmid);

/**********SEMAPHORE OPS******************************************************/
key_t SemCreate(int shkey);
int SemGet(key_t semkey);
void SemDown(int semid);
void SemUp(int semid);
void SemDelete(int semid);
void SemOp(int semid, int value);
#endif
