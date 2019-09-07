#ifndef OS_PROCESSP_H
#define OS_PROCESSP_H

#include "ErrorCodes.h"
#include "InQueue.h"
#include "OutQueue.h"


int ProcessP(char *input_filepath, InQueueHeader *in_queue, OutQueueHeader *out_queue);

void ReadRandomLine(char **message_ptr, size_t *message_size);

char *ConstructPayload(InMessageHeader msg_header, char *message);

#endif //OS_PROCESSP_H
