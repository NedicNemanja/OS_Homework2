#include "ErrorCodes.h"
#include "InQueue.h"
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
int InQueueInit(key_t shkey, size_t memSize) { //TODO fix this one
    //get semaphore keys
    //printf("Getting semaphore keys for SharedMemoryQueue %d:\n", (int)shkey);
    key_t semkey_msgNum = SemCreate((int) shkey);
    key_t semkey_freeSpace = SemCreate((int) shkey + 1);
    //initialize free space semaphore
    SemOp(SemGet(semkey_freeSpace), memSize);
    //get memory segment
    int shmid;
    if ((shmid = shmget(shkey, sizeof(SHMemQueue) + memSize, IPC_CREAT | 0600)) < 0) {
        perror("shmget: ");
        exit(SHMGET);
    }
    //initialize shared memory segment
    char *mem = QueueAttach(shmid);
    InQueue inQueue = {sizeof(InQueue), sizeof(InQueue), sem_key,};
    //write queue data to shared memory
    memcpy(mem, &inQueue, sizeof(InQueue));
    QueueDetach(mem);
    return shmid;
}

void InQueue_Write(InQueueHeader *queue, char *payload, size_t payload_size) {
    //try to down writeSem, if fail wait for your turn to write
    SemDown(SemGet(queue->semkey_writeSem));
    //try to reduce freeSpace by the size of payload (message+header), if fail wait for payload to be read and freed
    SemOp(SemGet(queue->semkey_freeSpace), -payload_size);
    //write payload data into shared memory
    queue->end_ptr += payload_size; //update end_ptr
    char *dest = InQueue_GetPtr(queue, queue->end_ptr);
    memcpy(dest, payload, payload_size);
    //up msgNum semaphore by 1
    SemUp(SemGet(queue->semkey_msgNum));
    //up writeSem
    SemUp(SemGet(queue->semkey_writeSem));
}

char *InQueue_Read(InQueueHeader *queue) {
    //try to down msgNum, if fail wait for message to be written
    SemDown(SemGet(queue->semkey_msgNum));
    //get message header from start_ptr (to find out the message size)
    InMessageHeader msg_header = InMessage_GetHeader(queue);
    //now that we know the message size, get message from queue
    char *message_start_ptr = InQueue_GetPtr(queue, queue->start_ptr) + sizeof(InMessageHeader);
    char *message = malloc(msg_header.message_size);
    memcpy(message, message_start_ptr, msg_header.message_size);
    //got the message, update start_ptr
    int payload_size = sizeof(InMessageHeader) + msg_header.message_size;
    queue->start_ptr += payload_size;
    //this payload space is now free, increase freeSpace semaphore
    SemOp(SemGet(queue->semkey_freeSpace), payload_size);
    return message;
}

char *InQueue_GetPtr(InQueueHeader *queue, int end_ptr) {
    return (char *) queue + sizeof(InQueueHeader) + end_ptr;
}

/*******************   InMessage   ********************************/

/**
 * Get the InMessageHeader of the message that starts at start_ptr of the queue
 * @param queue the queue on which we operate
 * @return InMessageHeader data structure
 */
InMessageHeader InMessage_GetHeader(InQueueHeader *queue) {
    InMessageHeader msg_header;
    char *msg_start_ptr = InQueue_GetPtr(queue, queue->start_ptr);
    memcpy(&msg_header, msg_start_ptr, sizeof(InMessageHeader));
    return msg_header;
}

