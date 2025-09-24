#ifndef __WORKERS_H__
#define __WORKERS_H__
#include <list>
#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>

struct Task {
    unsigned char priorityLevel = 0;

    virtual bool run() = 0;
    virtual ~Task() = default;
};

static constexpr size_t LEVELS = 5;
static constexpr size_t TIME_QUANTA[LEVELS] = { 1, 5, 10, 20 };
static constexpr std::chrono::milliseconds BOOST_QUANTA(60);

class MLFQ {
    std::list<std::unique_ptr<Task>> _levels[LEVELS];
    std::atomic<size_t> size;

    std::mutex qMtx[LEVELS]; // Later switch to lock free

    std::mutex emptyMtx;
    std::condition_variable signalNonEmpty;

    using clock = std::chrono::steady_clock;
    using rep   = clock::duration::rep;

    std::atomic< rep > lastBoost;

public:
    MLFQ();

    // Delete these for now; it'll be static so we leave it alone
    MLFQ(const MLFQ& other) = delete;
    MLFQ(MLFQ&& other) = delete;
    MLFQ& operator=(const MLFQ& other) = delete;
    MLFQ& operator=(MLFQ&& other) = delete;

    void enqueue( std::unique_ptr<Task> task );

    // Could return null - check for validity on return (since it's multithreaded)
    std::unique_ptr<Task> dequeue();

    bool checkQuanta();

    void boost();


};


#endif