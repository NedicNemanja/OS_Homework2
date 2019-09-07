//
// Created by nem on 04.09.19.
//

#include "ProcessP.h"

#include <unistd.h>
#include <stdio.h>

int ProcessP(char *input_filepath, InQueueHeader *in_queue, OutQueueHeader *out_queue) {
    int pid_match = 0;
    pid_t my_pid = getpid();
    //open input file
    FILE *f = fopen(input_filepath, "r");
    while (1) {
        //get random line from txt file
        char *message;
        size_t message_size;
        ReadRandomLine(&message, &message_size);
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
                return pid_match;
            }
        } while (response == NULL);
        pid_match++;
        printf("%d: %s\n", (int) my_pid, message);
    }
}

void ReadRandomLine(char **message_ptr, size_t *message_size) {
    *message_ptr = malloc(4);
    memcpy(message_ptr, "asd", 4);
    *message_size = 4;
}

char *ConstructPayload(InMessageHeader msg_header, char *message) {
    char *payload = malloc(sizeof(InMessageHeader) + msg_header.message_size + 1);
    memcpy(payload, &msg_header, sizeof(InMessageHeader));
    memccpy(payload + sizeof(InMessageHeader), message, msg_header.message_size);
    return payload;
}
