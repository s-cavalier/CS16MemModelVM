#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__
#include "kstl/Queue.h"
#include "Process.h"

namespace kernel {

    // Could change this later to adapt at runtime
    static constexpr uint32_t NUM_LEVELS = 3;
    static constexpr uint32_t QUANTUMS[NUM_LEVELS] = {4, 8, 16};
    
    class MultiLevelQueue {
        ministl::list<uint32_t> levels[NUM_LEVELS];
        size_t _size;
    
    public:
        MultiLevelQueue();

        static MultiLevelQueue scheduler;

        void enqueue(PCB::Guard proc);
        PCB::Guard dequeue();
        void boost();
        
        size_t size() const;
        bool empty() const;
        static constexpr uint32_t getQuantaForLevel(size_t i) { return QUANTUMS[i]; }
    };

}

#endif