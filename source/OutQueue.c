#include "OutQueue.h"
#include "Globals.h"
#include "SharedMem.h"

#include <unistd.h>
#include <stdio.h>
#include "string.h"

void OutQueue_Write(OutMessage message) {

}

char* OutQueue_Read(OutQueueHeader *queue) {
    //request read access from queue
    SemDown(SemGet(queue->semkey_read));
    //check if there are messages to be read
    SemDown((SemGet(queue->semkey_msgNum)));
    //get response
    char* response_ptr = OutQueue_GetPtr(queue);
    OutMessage response;
    memcpy(&response, response_ptr, sizeof(OutMessage));
    //check if this process is the receiver
    if(getpid() == response.pid) {
        //remove message from queue
        (queue->first)++;
        //release read rights for queue
        SemUp(SemGet(queue->semkey_read));
        return response.message;
    }
    else {
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
char* OutQueue_GetPtr(OutQueueHeader *queue) {
    return (char *) queue + sizeof(OutQueueHeader) + (queue->first*sizeof(OutMessage));
}