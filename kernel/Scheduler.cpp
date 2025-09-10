#include "Scheduler.h"

kernel::MultiLevelQueue::MultiLevelQueue() : levels(), _size(0) {}

void kernel::MultiLevelQueue::enqueue(PCB::Guard proc) {
    assert(proc->priority < NUM_LEVELS);
    if (proc.valid()) {
        levels[proc->priority].push_back(proc->getPID());
        ++_size;
    }
}

kernel::PCB::Guard kernel::MultiLevelQueue::dequeue() {
    uint32_t pid = -1;
    for (size_t i = 0; i < NUM_LEVELS; ++i) {
        if (levels[i].empty()) continue;
        pid = levels[i].front();
        levels[i].pop_front();
        --_size;
        break;
    }

    return ProcessManager::instance[pid];
}

void kernel::MultiLevelQueue::boost() {
    for (size_t i = 1; i < NUM_LEVELS; ++i) {
        while (!levels[i].empty()) {
            levels[0].push_back( levels[i].back() );
            levels[i].pop_back();
        } 
    }
}

size_t kernel::MultiLevelQueue::size() const {
    return _size;
}

bool kernel::MultiLevelQueue::empty() const {
    return _size == 0;
}

kernel::MultiLevelQueue kernel::MultiLevelQueue::scheduler;