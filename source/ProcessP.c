#include "ProcessP.h"
#include "SharedMem.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int ProcessP(char *input_filepath, int in_queue_id, int out_queue_id) {
    int pid_match = 0;
    pid_t my_pid = getpid();
    printf("P running with pid %d\n", my_pid);
    //attach shared memory
    InQueueHeader *in_queue = (InQueueHeader *) QueueAttach(in_queue_id);
    OutQueueHeader *out_queue = (OutQueueHeader *) QueueAttach(out_queue_id);
    //open input file
    int line_array_size;
    char** line_array = LoadFileToMemory(input_filepath, &line_array_size);
    srand(time(NULL));
    while (1) {
        /*************write*************************/
        //get random line from txt file
        char *message = line_array[rand()%line_array_size];
        size_t message_size = strlen(message)+1;
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
                QueueDetach((char *) in_queue);
                QueueDetach((char *) out_queue);
                FreeFile(line_array, line_array_size);
                return pid_match;
            }
        } while (response == NULL);
        pid_match++;
        printf("P %d read: %s\n", (int) my_pid, response);
        free(response);
    }
}

char **LoadFileToMemory(char* filepath, int* array_size_ptr) {
    char** line_array = NULL;
    int line_array_size=0;

    FILE *fp = fopen(filepath, "r");
    if (fp == NULL)
        exit(FOPEN_FAIL);

    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1) {
        if(read <= 1) {
            free(line);
            line = NULL;
            len = 0;
            continue;
        }
        line[read-1] = '\0';
        line_array_size++;
        line_array = realloc(line_array, line_array_size*sizeof(char*));
        line_array[line_array_size-1] = malloc(strlen(line)+1);
        strcpy(line_array[line_array_size-1], line);
        free(line);
        line = NULL;
        len = 0;
    }
    if(line != NULL)
        free(line);
    fclose(fp);
    *array_size_ptr = line_array_size;
    return line_array;
}

void PrintFile(char** line_array, int line_array_size) {
    for(int i=0; i<line_array_size; i++) {
        printf("%s", line_array[i]);
    }
}

void FreeFile(char** line_array, int line_array_size) {
    for(int i=0; i<line_array_size; i++) {
        free(line_array[i]);
    }
    free(line_array);
}

char *ConstructPayload(InMessageHeader msg_header, char *message) {
    char *payload = malloc(sizeof(InMessageHeader) + msg_header.message_size);
    memcpy(payload, &msg_header, sizeof(InMessageHeader));
    memcpy(payload + sizeof(InMessageHeader), message, msg_header.message_size);
    return payload;
}
