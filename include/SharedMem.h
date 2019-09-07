#ifndef SHAREDMEM_H
#define SHAREDMEM_H

#include <sys/types.h>

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

typedef struct SHMemQueue{
    volatile int back; //newest queue element
    volatile int next; //next elem to be retrieved
    key_t semkey;
} SHMemQueue;

/***********Initialization and Destruction*************************************/
/*Get size of queue memory for queue elements (in bytes)*/
size_t MEMSIZE(int num_parts);

/*Get shared memory segment and semaphore keys with ftok*/
int QueueInit(key_t shkey,size_t size);
/*Attach shared memory segment*/
char* QueueAttach(int shmid);
void QueueDeleteSemaphores(SHMemQueue* queue);
void QueueDetach(char* shm);
void QueueDelete(int shmid);

/************GETTERS***********************************************************/
/*return pointer to element at offset*/
char* GetOffset(SHMemQueue* queue, int offset);

/**********SEMAPHORE OPS******************************************************/
key_t SemCreate(int shkey);
int SemGet(key_t semkey);
void SemDown(int semid);
void SemUp(int semid);
void SemDelete(int semid);
void SemOp(int semid, int value);
#endif
