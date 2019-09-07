#ifndef OS_PROCESSC_H
#define OS_PROCESSC_H

void ProcessC(numIterations, numProcessP, allProcessP, sharedMemIn, sharedMemOut) {
    for(int i=0; i<numIterations; i++) {
        //down sem in InQueue
        //get InMessage from InQueue (this will mark that space as free)
        //MD5 hash in-ds.message
        //construct out-ds(in-ds.pid, hash)
        //lock out-ds for write
        //write out-ds
        //unlock out-ds
    }
    //send signal to allProcessP to halt
    //check that all of them received it (reuse same signal from P, count how many you got, or check if they are still alive?
}

#endif //OS_PROCESSC_H
