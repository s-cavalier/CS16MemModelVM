#ifndef __MANAGER_H__
#define __MANAGER_H__
#include "Process.h"
#include "VirtualMemory.h"
#include "HeapManager.h"

namespace kernel {

    class SharedResources {
        SharedResources();
    
    public:
        static SharedResources manager;

        FreeList freeList;
        MemoryManager memory;
        ProcessManager processes;

    };

    static inline SharedResources& sharedResources = SharedResources::manager;
    

}



#endif