#ifndef __PROCESS_H__
#define __PROCESS_H__
#include "ASMInterface.h"

namespace kernel {

    enum ProcessState {
        READY,
        RUNNING,
        BLOCKED,
        ZOMBIE
    };

    struct PCB {
        uint32_t pid_t;


    };


}

#endif