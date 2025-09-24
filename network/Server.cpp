#include "Server.h"
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <vector>
#include "WebSocket.h"
#include "../machine/KernelElf.h"

struct SendIO : public Hardware::stdIODevice {
    std::weak_ptr<Server::Connection> conn;

    SendIO( std::weak_ptr<Server::Connection> conn ) : conn(conn) {} 

    size_t read(char* buf, size_t n) override { /*no op for now*/ return 0; }
    void write(const std::string& data) override {
        auto ptr = conn.lock();
        if (!ptr) {
            return;
        }

        try {
            ptr->send( convertToPacket(data) );
        } catch (const std::runtime_error& e) {
            return;
        }
    }

};

Server::Connection::Connection( uint32_t sockfd, ConnectionList& container, ConnectionList::iterator self, MLFQ& scheduler ) 
    : handshakeStarted(false), sockfd(sockfd), container(&container), self(self), scheduler(scheduler), runningTask(nullptr) {
        vm.store( new Hardware::Machine(), std::memory_order_relaxed);
    }

Server::Connection::~Connection() {
    close(sockfd);

    Hardware::Machine* m = vm.exchange(nullptr, std::memory_order_acquire);
    if (m) delete m;

}
void Server::Connection::kill() {
    container->erase(self);
    std::cout << "Lost a connection." << std::endl;
}

void Server::Connection::send(const std::vector<char>& bytes) {
    size_t totalBytesSent = 0;
    while ( totalBytesSent < bytes.size() ) {
        int bytesSent = ::send(sockfd, bytes.data() + totalBytesSent , bytes.size() - totalBytesSent, 0);

        if (bytesSent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK) ) break;
        else if (bytesSent == -1) throw std::runtime_error("Bad error with loading data into conn " + std::to_string(sockfd));

        totalBytesSent += bytesSent;
    }
}

void Server::Connection::send(const std::string& bytes) {
    size_t totalBytesSent = 0;
    while ( totalBytesSent < bytes.size() ) {
        int bytesSent = ::send(sockfd, bytes.data() + totalBytesSent , bytes.size() - totalBytesSent, 0);

        if (bytesSent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK) ) break;
        else if (bytesSent == -1) throw std::runtime_error("Bad error with loading data into conn " + std::to_string(sockfd));

        totalBytesSent += bytesSent;
    }
}

template <size_t N>
void Server::Connection::send(const std::array<uint8_t, N>& bytes) {
    size_t totalBytesSent = 0;
    while ( totalBytesSent < bytes.size() ) {
        int bytesSent = ::send(sockfd, bytes.data() + totalBytesSent , bytes.size() - totalBytesSent, 0);

        if (bytesSent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK) ) break;
        else if (bytesSent == -1) throw std::runtime_error("Bad error with loading data into conn " + std::to_string(sockfd));

        totalBytesSent += bytesSent;
    }
}

void Server::Connection::dispatch(uint32_t event) {
    if ( (event & EPOLLERR) || (event & EPOLLHUP) ) {
        std::cout << "Connection with sockfd " << sockfd << " has died by " << event << '\n';
        kill();
        return;
    }

    if ( (event & EPOLLIN) == 0 ) throw std::runtime_error("Connection dispatch had no incoming data: " + std::to_string( event ) );

    std::vector<char> data;
    char buf[256];

    for (;;) {
        int res = recv(sockfd, buf, sizeof(buf), 0);
        
        if (res == 0) {
            kill();
            return;
        }

        if (res == -1 && (errno == EAGAIN || errno == EWOULDBLOCK) ) break;
        else if (res == -1) throw std::runtime_error("Bad error with loading data into conn " + std::to_string(sockfd));

        size_t oldsize = data.size();
        data.resize( oldsize + res );
        for (int i = 0; i < res; ++i) data[oldsize + i] = buf[i];
    }

    if (!handshakeStarted) {
        data.push_back('\0');

        std::cout << "Recieved a new WebSocket connection." << std::endl;

        std::string response;
        bool failed = false;
        
        try {
            response = buildWebSocketResponse( parseWebSocketRequest(data.data(), "/res") );
        } catch (const std::exception& e)  {
            std::cout << "Error: " << e.what() << std::endl;

            failed = true;
            response = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n";
        }

        send(response);

        if (failed) {
            kill();
            return;
        }

        handshakeStarted = true;
        return;
    }

    ClientToServerPacket packet(data);

    if (packet.payload.empty()) {
        send( disconnectHeader );
        return;
    }

    packet.payload.push_back('\0');

    std::unique_ptr<Task> newTask = generateTask( packet.payload.data() );

    if (!newTask) return;

    scheduler.enqueue( std::move(newTask) );

}

struct Forward : public Task {
    std::weak_ptr<Server::Connection> conn;
    std::string data;
 
    bool run() override { 
        auto ptr = conn.lock();
        if (!ptr) return false;
        ptr->send( convertToPacket(data) ); 
        return false; 
    }
};

struct Terminate : public Task {
    std::weak_ptr<Server::Connection> conn;
 
    bool run() override { 
        auto ptr = conn.lock();
        if (!ptr) return false;
        ptr->send( disconnectHeader ); 
        return false; 
    }
};

VMTask::~VMTask() {
    auto ptr = conn.lock();

    Hardware::Machine* m = vm.exchange(nullptr, std::memory_order_acquire);

    // If this task owns the VM and the Connection is still alive
    if (m && ptr) {
        if (exitmsg) ptr->send( convertToPacket(exitmsg) );
        ptr->vm.store(m, std::memory_order_release);
    }

    // If this task owns the VM and the Connection is not alive
    if (m && !ptr) {
        delete m;
    }

    // otherwise no-op
}

struct LoadKernel : public VMTask {
    bool completedInit{false};

    enum LoadState : char {
        TEXT,
        DATA,
        ARGUMENTS,
        COMPLETE
    } state{TEXT};

    Word writeIdx;


    LoadKernel(std::weak_ptr<Server::Connection>& conn ) {
        this->conn = conn;
        exitmsg = "[Server] Kernel loaded!";
    }

    bool run() override {
        auto ptr = conn.lock(); // It's possible to reclaim VM -> immediately disconnect, which will free m while this is running, so we need to keep conn alive
        if (!ptr) return false;

        Hardware::Machine* m = vm.load(std::memory_order_acquire);
        if (!m) return false; // Connection reclaimed ownership

        // Use a finite-state-machine model so we can iteratively build the kernel instead of as one blocking compute

        if ( !completedInit ) {
            m->trapEntry = KernelElf::trapEntry;
            m->cpu.programCounter = KernelElf::bootEntry;
            m->cpu.scu.registerFile[12].ui = 0b10;
            writeIdx = 0;
            m->stdio = std::make_unique<SendIO>(conn);

            completedInit = true;
        }

        if (state == TEXT) {
            // Use .data()[writeIdx] so we can leverage constexpr, since the opreator overload isn't constexpr
            m->memory.setByte(KernelElf::textStart + writeIdx, KernelElf::kernelText.data()[writeIdx], m->cpu.tlb, 0);
            ++writeIdx;

            // Switch writing state
            if (writeIdx >= KernelElf::kernelText.size()) {
                state = DATA;
                writeIdx = 0;
            }

            return true;
        }

        else if (state == DATA) {
            m->memory.setByte(KernelElf::dataStart + writeIdx, KernelElf::kernelData.data()[writeIdx], m->cpu.tlb, 0);
            ++writeIdx;

            // Switch writing state
            if (writeIdx >= KernelElf::kernelData.size()) {
                state = COMPLETE;
                return false;
            }

            return true;
        }

        return false;
    }

};

struct Step : public VMTask {
    size_t count;

    Step(std::weak_ptr<Server::Connection>& conn, size_t count) : count(count) {
        this->conn = conn;
        exitmsg = "[Server] Completed steps!";
    }

    bool run() override {
        if (count == 0) return false;

        auto ptr = conn.lock();
        if (!ptr) return false; // User disconnected
        Hardware::Machine* m = vm.load(std::memory_order_acquire);
        if (!m) return false; // VM reclaimed by Conn 

        if (m->killed) {
            ptr->send( convertToPacket("[Server] Program has run to completion!") );
            
            return false;
        }

        m->step();
        --count;
        return true;
    }

};

std::unique_ptr<Task> Server::Connection::generateTask(const std::string& str) {
    using TaskGenerator = std::unique_ptr<Task>(*)(std::weak_ptr<Server::Connection> conn, std::string_view view);

    size_t loc = str.find(' ');
    if (loc == std::string::npos) return nullptr;

    std::string command = str.substr(0, loc);
    std::string_view args( str.data() + loc + 1, str.size() - loc - 1 );


    // Since there's the conn pointer, you can just return nullptr to do a small-op, like a step
    #define TASK_GEN(key, func_body) { key, [] (std::weak_ptr<Server::Connection> conn, std::string_view view) -> std::unique_ptr<Task> func_body }
    static const std::unordered_map<std::string_view, TaskGenerator> factory{
        TASK_GEN("FORWARD", { std::unique_ptr<Forward> t = std::make_unique<Forward>(); t->conn = conn; t->data = view; return t; }),
        TASK_GEN("TERMINATE", { std::unique_ptr<Terminate> t = std::make_unique<Terminate>(); t->conn = conn; return t; }),
        TASK_GEN("LOAD_KERNEL", { 
            auto ptr = conn.lock();
            if (!ptr) return nullptr;

            Hardware::Machine* m = ptr->vm.exchange(nullptr, std::memory_order_acq_rel);
            if (!m) return nullptr;

            std::unique_ptr<LoadKernel> t = std::make_unique<LoadKernel>(conn);
            ptr->runningTask = t.get();
            t->vm.store(m, std::memory_order_relaxed);
            return t;
        }),
        TASK_GEN("STEP", {
            size_t count = std::atoi( view.data() );
            if (count == 0) return nullptr;

            auto ptr = conn.lock();
            if (!ptr) return nullptr;

            Hardware::Machine* m = ptr->vm.exchange(nullptr, std::memory_order_acq_rel);
            if (!m) return nullptr;

            std::unique_ptr<Step> t = std::make_unique<Step>(conn, count);
            ptr->runningTask = t.get();
            t->vm.store(m, std::memory_order_relaxed);
            return t;
        })
    };


    auto it = factory.find(command);
    if (it == factory.cend()) return nullptr;

    return it->second( *self , args );
}

void Server::dispatch(uint32_t event) {

    // Only should recieve EPOLLIN event. S
    if ( event != EPOLLIN ) throw std::runtime_error("Server dispatch recieved bad event type: " + std::to_string(event) );

    for (;;) {
        int new_fd = accept(listener, nullptr, nullptr); // For right now, we don't need to worry about incoming addresses. Maybe later.

        if (new_fd == -1 && (errno == EAGAIN || errno == EWOULDBLOCK )) return;
        else if (new_fd == -1) throw std::runtime_error("Bad error with accepting connections from listener.");

        auto& new_conn = connections.emplace_front();
        auto it = connections.begin();

        new_conn = std::make_shared<Connection>(new_fd, connections, it, scheduler);

        int flags = fcntl(new_fd, F_GETFL, 0);
        fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);

        epoll_event new_event;
        new_event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
        new_event.data.ptr = new_conn.get();

        epoll_ctl(epoller, EPOLL_CTL_ADD, new_fd, &new_event);
    }


}

Server::Server(const char* port, size_t numWorkers) {
    addrinfo hints;
    addrinfo* res;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICHOST ; // NUMERICHOST for connecting to localhost

    int status = getaddrinfo("127.0.0.1", port, &hints, &res);

    if ( status != 0 ) throw std::runtime_error(std::string("Failed to create getaddrinfo instance: ") + gai_strerror(status));

    listener = socket( res->ai_family , res->ai_socktype , res->ai_protocol );

    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    status = bind( listener, res->ai_addr, res->ai_addrlen );

    freeaddrinfo( res );

    if (status == -1 ) throw std::runtime_error("Failed to create bind instance.");

    status = ::listen(listener, 10);

    if (status == -1 ) throw std::runtime_error("Failed to create listener instance.");

    epoller = epoll_create1(0);

    if (epoller == -1) throw std::runtime_error("Failed to create epoller instance.");

    epoll_event listener_event;
    listener_event.events = EPOLLIN;
    listener_event.data.ptr = this;
    status = epoll_ctl(epoller, EPOLL_CTL_ADD, listener, &listener_event);

    int flags = fcntl(listener, F_GETFL, 0);
    fcntl(listener, F_SETFL, flags | O_NONBLOCK);

    if (status == -1) throw std::runtime_error("Failed to update epoll.");

    workers.reserve(numWorkers);
    for (size_t i = 0; i < numWorkers; ++i) workers.emplace_back( std::make_unique<std::thread>( [&] () {

        for (;;) {
            std::unique_ptr<Task> currentTask = scheduler.dequeue();
            if (!currentTask) {
                std::cout << "Somehow dequeue failed." << std::endl;
                continue;
            }

            auto sliceStart = std::chrono::steady_clock::now();
            auto timeLimit = std::chrono::milliseconds(TIME_QUANTA[ currentTask->priorityLevel ]);
            
            while (currentTask->run()) {
                auto now = std::chrono::steady_clock::now();
                if (now - sliceStart < timeLimit) continue;
                    
                scheduler.enqueue(std::move(currentTask));
                break;
            }

        }


    } ) );

}

void Server::listen() {
    std::vector<epoll_event> possible_events( connections.size() + 1 );

    for (;;) {

        size_t num_events = epoll_wait(epoller, possible_events.data(), possible_events.size(), -1);
        if (num_events == size_t(-1)) {
            perror("epoll_wait");
            throw std::runtime_error("Epoll wait failed...");
        }

        for (size_t i = 0; i < num_events; ++i) {
            uint32_t event_type = possible_events[i].events;
            ((EventListener*) possible_events[i].data.ptr )->dispatch(event_type);
        }

        possible_events.resize( connections.size() + 1 );
    }
}

Server::~Server() {
    close( listener );
    close( epoller );
}