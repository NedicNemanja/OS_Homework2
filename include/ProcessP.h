#ifndef OS_PROCESSP_H
#define OS_PROCESSP_H

#include "ErrorCodes.h"
#include "InQueue.h"
#include "OutQueue.h"


int ProcessP(char *input_filepath, int in_queue_id, int out_queue_id) ;

char *ConstructPayload(InMessageHeader msg_header, char *message);

char **LoadFileToMemory(char* filepath, int* array_size_ptr);

void FreeFile(char** line_array, int line_array_size);

void PrintFile(char** line_array, int line_array_size);

#endif //OS_PROCESSP_H
