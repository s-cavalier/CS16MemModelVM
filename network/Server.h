#ifndef __SERVER_H__
#define __SERVER_H__
#include <cstdint>
#include <cstring>
#include <list>
#include <memory>
#include <string_view>
#include <atomic>
#include "Workers.h"
#include "../machine/Hardware.h"
#include "../loader/Loader.h"

struct EventListener {

    virtual void dispatch(uint32_t event) = 0;
    virtual ~EventListener() = default;

};

struct VMTask;

class Server : public EventListener {
public: 
    class Connection;
    using ConnectionList = std::list< std::shared_ptr<Connection> >;

private:

    uint32_t listener;
    int32_t epoller;
    ConnectionList connections;
    MLFQ scheduler;
    std::vector< std::unique_ptr<std::thread> > workers;    

    void dispatch(uint32_t event) override;

public:

    class Connection final : public EventListener {
        bool handshakeStarted;
        uint32_t sockfd;
        ConnectionList* container;
        ConnectionList::iterator self; // Has an iterator and pointer to self and container to remove itself
        MLFQ& scheduler;

    public:
        VMTask* runningTask; 
        // Every connection should have a raw ptr to it's current (and only) VMTask
        // It needs to be able to reclaim the VM at any time from the task, and so the atomic ptr buys us Atomic ownership;
        // only one thread can own the pointer. And thus either the thread the task is on or the main thread owns the VM pointer at any point in time
        // How do we know if runningTask is dangling then? We know because either we own the VM ptr, xor it does, not both
        // And so we know that runningTask owns the ptr implies it exists
        // If we own the pointer, runningTask may or not be valid, but we only need the pointer to reclaim the VM, so this is fine

        // TLDR; vm.load(...) == nullptr means runningTask is a valid pointer
        // otherwise, runningTask may or not be valid

        std::atomic<Hardware::Machine*> vm;

        Connection( uint32_t sockfd, ConnectionList& container, ConnectionList::iterator self, MLFQ& scheduler );
        ~Connection();

        void kill();
        void send(const std::string& str);
        void send(const std::vector<char>& str);

        template <size_t N>
        void send(const std::array<uint8_t, N>& str);

        virtual void dispatch(uint32_t event) override;

        Connection( Connection&& other ) = delete;
        Connection(const Connection& other) = delete;
        Connection& operator=(Connection&& other) = delete;
        Connection& operator=(const Connection& other) = delete;

        std::unique_ptr<Task> generateTask(const std::string& str);

    };

    Server(const char* port = "3940", size_t numWorkers = std::thread::hardware_concurrency());
    ~Server();

    void listen();

    // Delete these for now; maybe change later
    Server(const Server& other) = delete;
    Server(Server&& other) = delete;
    Server& operator=(const Server& other) = delete;
    Server& operator=(Server&& other) = delete;

    
};

struct VMTask : public Task {
    std::atomic<Hardware::Machine*> vm;
    std::weak_ptr<Server::Connection> conn;
    const char* exitmsg;

    bool run() override = 0;

    ~VMTask() override;

};

#endif
