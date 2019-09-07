#include "ErrorCodes.h"
#include "SharedMem.h"
#include "InQueue.h"
#include "OutQueue.h"
#include "Globals.h"
#include "ProcessC.h"
#include "ProcessP.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h> //wait for children
#include <unistd.h> //fork
#include <string.h> //strcmp

void ParseArguments(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    ParseArguments(argc, argv);
    //create 2 shared_mem segments, one for in one for out
    key_t memkeys[2] = {0x1111, 0x2222};
    int in_queue_id = InQueue_Init(0x1111, IN_QUEUE_SIZE);
    int out_queue_init = OutQueue_Init(0x2222, OUT_QUEUE_SIZE)};

    pid_t *processPArray = malloc(NUM_P *sizeof(pid_t));
    for (int i = 0; i < NUM_P; i++) {
        processPArray[i] = fork();
        if (processPArray[i] == 0) { //if child
            return ProcessP(sharedMemIn, sharedMemOut);
        }
    }
    pid_t pid_C = fork();
    if (pid_C == 0) {
        return ProcesssC(numIterations, numProcessP, processPArray, sharedMemIn, sharedMemOut);
    }

    //wait for all children before you terminate
    int status;
    while (wait(&status) > 0);
    printf("Children done\n");

    //TODO how many pid_match for each P, collect return status if child?
    //TODO print stats

    /*Cleanup*/
    for(int i=0; i<3; i++){
        //delete shared memory
        QueueDelete(queueids[i]);
    }
    return 0;
}

void ParseArguments(int argc, char *argv[]) {
    int arg_index = 1;
    int N_flag = 0, K_flag = 0;
    if (argc != 5) {
        fprintf(stderr, "Expected 5 cmd arguments. Got: \n", argc);
        exit(BAD_CMDARGUMENTs);
    }

    //scan all arguments to find (-n,-i)
    while (arg_index < argc) {
        //make sure there is an argument to check
        if (argv[arg_index] == NULL)
            break;

        if (strcmp(argv[arg_index], "-n") == 0 && N_flag == 0) {
            NUM_P = atoi(argv[++arg_index]);
            N_flag = 1;
            arg_index++;
            continue;
        }
        if (strcmp(argv[arg_index], "-i") == 0 && K_flag == 0) {
            NUM_ITERATIONS = atoi(argv[++arg_index]);
            arg_index++;
            K_flag = 1;
            continue;
        }
        fprintf(stderr, "Unknown argument: %s\n", argv[arg_index]);
        exit(BAD_CMDARGUMENTs);
    }

    //check that you got all the arguements
    if (N_flag == 0) {
        fprintf(stderr, "Specify number of P processes using \"-n num_procP\".\n");
        exit(NO_NUM_P);
    }
    if (K_flag == 0) {
        fprintf(stderr, "Specify number of iterations/messages using \"-i port_num\".\n");
        exit(NO_NUM_ITERATIONS);
    }
}


