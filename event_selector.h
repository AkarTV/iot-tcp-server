#ifndef SOCKET_H
#define SOCKET_H

#include <vector>

const int expected_connections = 15; //for efficiency (reserve in vector)

class FdHandler {
    //File descriptor handler
    public:
    FdHandler(int fd_) : fd(fd_) {}
    virtual ~FdHandler();
    virtual void handle() = 0;
    int get_fd() const {return fd;}

    private:
    int fd; 
};

class EventSelector {
    //Stores all descriptors and runs the endless cycle for handling descriptors
    public:
    EventSelector();
    ~EventSelector();
    void add(FdHandler* handler);
    void remove(FdHandler* handler);
    void run(); //main loop

    private:
    std::vector<FdHandler*> descriptors;
    int max_fd = 0;
};

#endif