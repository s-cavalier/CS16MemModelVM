#include "Workers.h"
#include <stdexcept>
#include <iostream>

MLFQ::MLFQ() : size(0), lastBoost( std::chrono::steady_clock::now().time_since_epoch().count() ) {}

void MLFQ::enqueue(std::unique_ptr<Task> task) {
    if (!task) throw std::runtime_error("Empty task passed to enqueue");

    bool res = checkQuanta();

    if (res) task->priorityLevel = 0;
    else task->priorityLevel %= LEVELS;

    {
        std::lock_guard<std::mutex> lock( qMtx[task->priorityLevel] );
        _levels[ task->priorityLevel ].emplace_back( std::move(task) );
    }
    
    size.fetch_add(1, std::memory_order_relaxed);

    signalNonEmpty.notify_one();

}

std::unique_ptr<Task> MLFQ::dequeue() {
    checkQuanta();

    std::unique_ptr<Task> res;

    std::unique_lock<std::mutex> lock(emptyMtx);
    signalNonEmpty.wait(lock, [&] () { return size.load(std::memory_order_relaxed) > 0; } );

    for (size_t i = 0; i < LEVELS; ++i) {
        std::lock_guard<std::mutex> lock( qMtx[ i ] );

        if ( _levels[i].empty() ) continue;

        res = std::move(_levels[i].front());
        _levels[i].pop_front();
        size.fetch_sub(1, std::memory_order_relaxed);
        break;
    }

    ++res->priorityLevel;

    return res;

}

bool MLFQ::checkQuanta() {
    auto now = clock::now();
    rep prev = lastBoost.load(std::memory_order_relaxed);
    auto prev_tp = clock::time_point(clock::duration(prev));

    if (now - prev_tp < BOOST_QUANTA) return false;
    
    // Try to claim the boost
    rep expected = prev;
    rep desired  = now.time_since_epoch().count();
    if (lastBoost.compare_exchange_strong(expected, desired)) {
        boost();
    }

    return true;
    
}

void MLFQ::boost() {

    std::lock_guard<std::mutex> boostlock( qMtx[0] );

    for (size_t i = 1; i < LEVELS; ++i) {
        std::lock_guard<std::mutex> lock( qMtx[i] );

        while (!_levels[i].empty()) {
            _levels[0].push_back( std::move( _levels[i].back() ) );
            _levels[0].back()->priorityLevel = 0;
            _levels[i].pop_back();
        }
    }
}