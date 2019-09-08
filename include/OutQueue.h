#ifndef OS_OUTQUEUE_H
#define OS_OUTQUEUE_H

#include "Globals.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>

typedef struct OutMessage {
    pid_t pid;
    char message[MD5_HASH_SIZE];
} OutMessage;


typedef struct OutQueueHeader {
    volatile int first;  //next message to be read from consumer. Set to 0 at start.
    volatile int last;  //last message in the queue. Set to -1 at start.
    key_t semkey_read;  //binary semaphore that controls who reads next from the queue. Set to 1 at start.
    key_t semkey_msgNum;//semaphore that gives the number of messages available for reading in queue. Set to 0 at start.
} OutQueueHeader;

int OutQueue_Init(key_t shkey, size_t memSize);

void OutQueue_Write(OutQueueHeader *queue, OutMessage message);

char *OutQueue_Read(OutQueueHeader *queue);

void OutQueue_DelSemaphores(OutQueueHeader *queue);

char *OutQueue_GetPtr(OutQueueHeader *queue, int i);

#endif //OS_OUTQUEUE_H
