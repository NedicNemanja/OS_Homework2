#ifndef OS_PROCESSP_H
#define OS_PROCESSP_H

#include "ErrorCodes.h"
#include "InQueue.h"
#include "OutQueue.h"


int ProcessP(char *input_filepath, int in_queue_id, int out_queue_id) ;

char *ReadRandomLine(size_t *message_size);

char *ConstructPayload(InMessageHeader msg_header, char *message);

#endif //OS_PROCESSP_H
