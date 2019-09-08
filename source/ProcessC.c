#include <InQueue.h>
#include <OutQueue.h>
#include "ProcessC.h"
#include "SharedMem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <sys/wait.h>

int ProcessC(int num_iterations, pid_t *processP_array, int num_processP, int in_queue_id, int out_queue_id) {
    printf("C running with pid %d\n", getpid());
    //attach shared mem segments
    InQueueHeader *in_queue = (InQueueHeader *) QueueAttach(in_queue_id);
    OutQueueHeader *out_queue = (OutQueueHeader *) QueueAttach(out_queue_id);
    for (int i = 0; i < num_iterations; i++) {
        //get a message from in_queue
        pid_t writer_pid;
        printf("C reading from InQueue\n");
        char *message = InQueue_Read(in_queue, &writer_pid);
        printf("C read: \"%s\"\n", message);
        //calculate MD5 hash
        char *hash = str2md5(message, strlen(message));
        //write hash to out_queue
        OutMessage response;
        response.pid = writer_pid;
        strcpy(response.message, hash);
        printf("C writing to OutQueue: {%d, %s}\n", writer_pid, response.message);
        OutQueue_Write(out_queue, response);
        //clean up
        free(message);
        free(hash);
    }
    //send message to all p processes to stop
    StopAll(processP_array, num_processP, out_queue);
    //cleanup
    //InQueue_DelSemaphores(in_queue);
    //OutQueue_DelSemaphores(out_queue);
    QueueDetach((char *) in_queue);
    QueueDetach((char *) out_queue);
    free(processP_array);
    return 0;
}

void StopAll(pid_t *processP_array, int num_processP, OutQueueHeader *out_queue) {
    for (int i = 0; i < num_processP; i++) {
        printf("C telling %d to terminate.\n", processP_array[i]);
        OutMessage message = {processP_array[i], "TERMINATE"};
        OutQueue_Write(out_queue, message);
    }
}

/**
 * https://stackoverflow.com/questions/7627723/how-to-create-a-md5-hash-of-a-string-in-c
 */
char *str2md5(const char *str, int length) {
    int n;
    MD5_CTX c;
    unsigned char digest[16];
    char *out = (char *) malloc(33);

    MD5_Init(&c);

    while (length > 0) {
        if (length > 512) {
            MD5_Update(&c, str, 512);
        } else {
            MD5_Update(&c, str, length);
        }
        length -= 512;
        str += 512;
    }

    MD5_Final(digest, &c);

    for (n = 0; n < 16; ++n) {
        snprintf(&(out[n * 2]), 16 * 2, "%02x", (unsigned int) digest[n]);
    }
    return out;
}
