#include "ProcessP.h"
#include "SharedMem.h"

#include <unistd.h>
#include <stdio.h>

int ProcessP(char *input_filepath, int in_queue_id, int out_queue_id) {
    int pid_match = 0;
    pid_t my_pid = getpid();
    printf("P running with pid %d\n", my_pid);
    //attach shared memory
    InQueueHeader *in_queue = (InQueueHeader *) QueueAttach(in_queue_id);
    OutQueueHeader *out_queue = (OutQueueHeader *) QueueAttach(out_queue_id);
    //open input file
    FILE *f = fopen(input_filepath, "r");
    while (1) {
        /*************write*************************/
        //get random line from txt file
        size_t message_size;
        char *message = ReadRandomLine(&message_size); //TODO make sure message_size is strlen()+1
        //construct and write InMessageHeader
        InMessageHeader msg_header = {message_size, my_pid};
        char *payload = ConstructPayload(msg_header, message);
        //insert payload to InQueue
        printf("P %d writing: \"%s\"...\n", my_pid, message);
        InQueue_Write(in_queue, payload, sizeof(InMessageHeader) + msg_header.message_size);
        printf("P %d done writing.\n", my_pid);
        free(payload);
        /***********read****************************/
        char *response;
        do {
            //get response from C
            printf("P %d reading...\n", my_pid);
            response = OutQueue_Read(out_queue);
            if (response != NULL && strcmp(response, "TERMINATE") == 0) {
                printf("P %d terminating with %d.\n", my_pid, pid_match);
                //cleanup
                free(response);
                fclose(f);
                QueueDetach((char *) in_queue);
                QueueDetach((char *) out_queue);
                return pid_match;
            }
        } while (response == NULL);
        pid_match++;
        printf("P %d read: %s\n", (int) my_pid, response);
        free(response);
    }
}

char *ReadRandomLine(size_t *message_size) {
    *message_size = 4;//TODO rewrite this funciton
    return "asd";
}

char *ConstructPayload(InMessageHeader msg_header, char *message) {
    char *payload = malloc(sizeof(InMessageHeader) + msg_header.message_size);
    memcpy(payload, &msg_header, sizeof(InMessageHeader));
    memcpy(payload + sizeof(InMessageHeader), message, msg_header.message_size);
    return payload;
}
