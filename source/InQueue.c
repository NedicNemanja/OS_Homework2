#include "ErrorCodes.h"
#include "InQueue.h"
#include "SharedMem.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h> //memcpy
#include <sys/sem.h>

/*Get shared memory segment*/
int InQueue_Init(key_t shkey, size_t memSize) {
    printf("Initialize InQueue to %d bytes\n", (int)memSize);
    //set up semaphores
    key_t semkey_writeSem = SemCreate((int) shkey);
    SemUp(SemGet(semkey_writeSem));
    printf("InQueue->semkey_writeSem set to %d\n", semctl(SemGet(semkey_writeSem),0,GETVAL));

    key_t semkey_msgNum = SemCreate((int) shkey+1);
    printf("InQueue->semkey_msgNum set to %d\n", semctl(SemGet(semkey_msgNum),0,GETVAL));

    key_t semkey_freeSpace = SemCreate((int) shkey + 2);
    SemOp(SemGet(semkey_freeSpace), memSize);
    printf("InQueue->semkey_freeSpace %d set to %d\n", (int)semkey_freeSpace, semctl(SemGet(semkey_freeSpace),0,GETVAL));

    //set up shared memory segment
    int shmid;
    if ((shmid = shmget(shkey, sizeof(InQueueHeader) + memSize, IPC_CREAT | 0600)) < 0) {
        perror("shmget: ");
        exit(SHMGET);
    }
    //initialize shared memory segment
    char *mem = QueueAttach(shmid);
    InQueueHeader in_queue = {0, 0, semkey_writeSem, semkey_msgNum, semkey_freeSpace};
    //write queue data to shared memory
    memcpy(mem, &in_queue, sizeof(InQueueHeader));
    QueueDetach(mem);
    return shmid;
}

void InQueue_Write(InQueueHeader *queue, char *payload, size_t payload_size) {
    //try to down writeSem, if fail wait for your turn to write
    printf("%d here0\n", getpid());
    SemDown(SemGet(queue->semkey_writeSem));
    printf("%d here\n", getpid());
    //check if there is enough freeSpace for the size of payload, if not wait for space to be freed by C
    SemOp(SemGet(queue->semkey_freeSpace), -payload_size);
    //write payload data into shared memory
    char *dest = InQueue_GetPtr(queue, queue->write_ptr);
    memcpy(dest, payload, payload_size);
    queue->write_ptr += payload_size; //update end_ptr
    //up msgNum semaphore by 1
    SemUp(SemGet(queue->semkey_msgNum));
    //up writeSem
    SemUp(SemGet(queue->semkey_writeSem));
}

char *InQueue_Read(InQueueHeader *queue, pid_t *writer_pid) {
    //try to down msgNum, if fail wait for message to be written
    SemDown(SemGet(queue->semkey_msgNum));
    //get message header from read_ptr (to find out the message size)
    InMessageHeader msg_header = InMessage_GetHeader(queue);
    *writer_pid = msg_header.pid;
    //now that we know the message size, get message from queue
    char *message_start_ptr = InQueue_GetPtr(queue, queue->read_ptr) + sizeof(InMessageHeader);
    char *message = malloc(msg_header.message_size);
    memcpy(message, message_start_ptr, msg_header.message_size);
    //got the message, update read_ptr
    int payload_size = sizeof(InMessageHeader) + msg_header.message_size;
    queue->read_ptr += payload_size;
    //this payload space is now free, increase freeSpace semaphore
    SemOp(SemGet(queue->semkey_freeSpace), payload_size);
    return message;
}

char *InQueue_GetPtr(InQueueHeader *queue, int end_ptr) {
    return (char *) queue + sizeof(InQueueHeader) + end_ptr;
}

void InQueue_DelSemaphores(InQueueHeader *queue) {
    SemDelete(SemGet(queue->semkey_writeSem));
    SemDelete(SemGet(queue->semkey_msgNum));
    SemDelete(SemGet(queue->semkey_freeSpace));
}


/*******************   InMessage   ********************************/

/**
 * Get the InMessageHeader of the message that starts at read_ptr of the queue
 * @param queue the queue on which we operate
 * @return InMessageHeader data structure
 */
InMessageHeader InMessage_GetHeader(InQueueHeader *queue) {
    InMessageHeader msg_header;
    char *msg_start_ptr = InQueue_GetPtr(queue, queue->read_ptr);
    memcpy(&msg_header, msg_start_ptr, sizeof(InMessageHeader));
    return msg_header;
}

