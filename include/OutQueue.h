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

size_t OUT_QUEUE_SIZE = (size_t) (MAX_NUM_OUT_MSG * sizeof(OutMessage));

typedef struct OutQueueHeader {
    volatile int first;  //next message to be read from consumer
    volatile int last;  //last message in the queue
    key_t semkey_read;  //binary semaphore that controls who reads next from the queue
    key_t semkey_msgNum;    //seamphre that gives the number of messages available for reading in queue
} OutQueueHeader;

void OutQueue_Write(OutMessage message);

char* OutQueue_Read(OutQueueHeader *queue);

/**
 * Get the pointer of the next message to be read from queue
 * @param queue
 * @return char* pointer to next OutMessage to be read
 */
char* OutQueue_GetPtr(OutQueueHeader *queue);

#endif //OS_OUTQUEUE_H
