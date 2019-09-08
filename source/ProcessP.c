#include "ProcessP.h"
#include "SharedMem.h"

#include <unistd.h>
#include <stdio.h>

int ProcessP(char *input_filepath, int in_queue_id, int out_queue_id) {
    int pid_match = 0;
    pid_t my_pid = getpid();
    printf("P running with pid %d\n", my_pid);
    //attach shared memory
    InQueueHeader* in_queue = (InQueueHeader*)QueueAttach(in_queue_id);
    OutQueueHeader* out_queue = (OutQueueHeader*)QueueAttach(out_queue_id);
    //open input file
    FILE *f = fopen(input_filepath, "r");
    while (1) {
        //get random line from txt file
        char *message;
        size_t message_size;
        ReadRandomLine(&message, &message_size); //TODO make sure message_size is strlen()+1
        //construct and write InMessageHeader
        InMessageHeader msg_header = {message_size, my_pid};
        char *payload = ConstructPayload(msg_header, message);
        //insert payload to InQueue
        InQueue_Write(in_queue, payload, sizeof(InMessageHeader) + msg_header.message_size);
        free(payload);
        char *response;
        do {
            //get response from C
            response = OutQueue_Read(out_queue);
            if (strcmp(response, "TERMINATE") == 0) {
                free(response);
                return pid_match;
            }
        } while (response == NULL);
        pid_match++;
        printf("%d read: %s\n", (int) my_pid, response);
        free(response);
    }
}

void ReadRandomLine(char **message_ptr, size_t *message_size) {
    *message_ptr = malloc(4);
    memcpy(message_ptr, "asd", 4);
    *message_size = 4;//TODO rewrite this funciton
}

char *ConstructPayload(InMessageHeader msg_header, char *message) {
    char *payload = malloc(sizeof(InMessageHeader) + msg_header.message_size + 1);
    memcpy(payload, &msg_header, sizeof(InMessageHeader));
    memcpy(payload + sizeof(InMessageHeader), message, msg_header.message_size);
    return payload;
}
