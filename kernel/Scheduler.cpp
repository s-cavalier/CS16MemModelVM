#include "Scheduler.h"

kernel::MultiLevelQueue::MultiLevelQueue() : levels(), _size(0) {}

void kernel::MultiLevelQueue::enqueue(PCB* proc) {
    assert(proc->priority < NUM_LEVELS);
    if (proc) {
        levels[proc->priority].push_back(proc->getPID());
        ++_size;
    }
}

kernel::uint32_t kernel::MultiLevelQueue::dequeue() {
    for (size_t i = 0; i < NUM_LEVELS; ++i) {
        if (levels[i].empty()) continue;
        uint32_t pid = levels[i].front();
        levels[i].pop_front();
        --_size;
        return pid;
    }

    return NOPCBEXISTS;
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