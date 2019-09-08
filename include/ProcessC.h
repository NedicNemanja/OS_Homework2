#ifndef OS_PROCESSC_H
#define OS_PROCESSC_H

int ProcessC(int num_iterations, int *processP_array, int num_processP, int in_queue_id, int out_queue_id);

void StopAll(int *processP_array, int num_processP, OutQueueHeader *out_queue);

char *str2md5(const char *str, int length);

#endif //OS_PROCESSC_H
