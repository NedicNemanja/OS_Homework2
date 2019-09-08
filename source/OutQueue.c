#include "ErrorCodes.h"
#include "OutQueue.h"
#include "Globals.h"
#include "SharedMem.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h> //memcpy


int OutQueue_Init(key_t shkey, size_t memSize) {
    printf("Initialize OutQueue\n");
    //get keys
    key_t semkey_read = SemCreate((int) shkey);
    SemUp(SemGet(semkey_read));
    key_t semkey_msgNum = SemCreate((int) shkey + 1);
    //get memory segment
    int shmid;
    if ((shmid = shmget(shkey, sizeof(OutQueueHeader) + memSize, IPC_CREAT | 0600)) < 0) {
        perror("shmget: ");
        exit(SHMGET);
    }
    //initialize queue data
    char *mem = QueueAttach(shmid);
    OutQueueHeader queue = {0, -1, semkey_read, semkey_msgNum};
    //write queue data to shared memory
    memcpy(mem, &queue, sizeof(OutQueueHeader));
    QueueDetach(mem);
    return shmid;
}


void OutQueue_Write(OutQueueHeader *queue, OutMessage message) {
    (queue->last)++;
    char *dest = OutQueue_GetPtr(queue, queue->last);
    memcpy(dest, &message, sizeof(OutMessage));
    SemUp(SemGet(queue->semkey_msgNum));
}

char *OutQueue_Read(OutQueueHeader *queue) {
    //request read access from queue
    SemDown(SemGet(queue->semkey_read));
    //check if there are messages to be read
    SemDown((SemGet(queue->semkey_msgNum)));
    //get response
    char *response_ptr = OutQueue_GetPtr(queue, queue->first);
    OutMessage response;
    memcpy(&response, response_ptr, sizeof(OutMessage));
    //check if this process is the receiver
    if (getpid() == response.pid) {
        //remove message from queue
        (queue->first)++;
        //release read rights for queue
        SemUp(SemGet(queue->semkey_read));
        //dont return ptr to local variable
        char* message = malloc(strlen(response.message)+1);
        memcpy(message, response.message, strlen(response.message)+1);
        return message;
    } else {
        //message not for me, release read rights, let the next process read the message
        SemUp(SemGet(queue->semkey_read));
        SemUp((SemGet(queue->semkey_msgNum)));
        return NULL;
    }
}

/**
 * Get the pointer of the next message to be read from queue
 * @param queue
 * @return char* pointer to next OutMessage to be read
 */
char *OutQueue_GetPtr(OutQueueHeader *queue, int i) {
    return (char *) queue + sizeof(OutQueueHeader) + (i * sizeof(OutMessage));
}


void OutQueue_DelSemaphores(OutQueueHeader *queue) {
    SemDelete(SemGet(queue->semkey_read));
    SemDelete(SemGet(queue->semkey_msgNum));
}