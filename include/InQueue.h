#ifndef OS_INQUEUE_H
#define OS_INQUEUE_H

#include "Globals.h"

#include <string.h> //memcpy
#include <unistd.h>
#include <sys/types.h>

/**
 * This header exists before each message data block. It is there to provide information about the data and its size.
 */
typedef struct InMessageHeader {
    size_t message_size;    //size of the string message including \0
    pid_t pid;
} InMessageHeader;

/**
 * Processes P write to this Queue and process C reads from it.
 * The InQueueHeader is included in the start of the Shared Memory segment.
 * Upon initialization the start_ptr and end_ptr both point after the InQueueHeader.
 * When a write is performed:
 *  The writer will check:
 *  -If some other process is already writing to the Queue (writeSem is down). If yes then wait (writeSem up).
 *  -If there is enough space to write the messageHeader+message, by trying to decrease the freeSpace
 *  semaphore by (sizeof(InMessageHeader)+msg_size).
 *  After successfully writing the data the msgNum semaphore will be upped by 1 as well and the end_ptr will be moved.
 * When a read is performed:
 *  -The reader will check if there are available messages to read (down the writeSem by 1)
 *  -When the message becomes available, the data is copied by the C process, and the freeSpace semaphore semval is
 *   increased in order to represent that the space has been read, and freed. With this action the start_ptr will also be moved.
 * Wrapping: when a message cannot fit in one block we will have end_ptr <= start_ptr and break it up into two writes
 *  and two reads for that message.
 */
typedef struct InQueueHeader {
    volatile int read_ptr; //points to where the allocated space of InQueue starts, in bytes. Set to 0 at start.
    volatile int write_ptr;   //points to where the allocated space of InQueue ends, in bytes. Set to 0 at start.
    key_t semkey_writeSem;  //down when someone is writting to the queue, upped once they finish. Set to 1 at start.
    key_t semkey_msgNum;   //upped when msg inserted, downed when msg removed from queue. Set to 0 at start.
    key_t semkey_freeSpace; //value of this sem reveals the available space in queue, in bytes.
} InQueueHeader;

int InQueue_Init(key_t shkey, size_t memSize);

void InQueue_Write(InQueueHeader *queue, char *payload, size_t payload_size);

char* InQueue_Read(InQueueHeader *queue, pid_t *writer_pid);

char *InQueue_GetPtr(InQueueHeader *queue, int end_ptr);

void InQueue_DelSemaphores(InQueueHeader *queue);

/*******************   InMessage   ********************************/

/**
 * Get the InMessageHeader of the message that starts at start_ptr of the queue
 * @param queue the queue on which we operate
 * @return InMessageHeader data structure
 */
InMessageHeader InMessage_GetHeader(InQueueHeader *queue);

#endif //OS_INQUEUE_H
