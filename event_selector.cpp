#include <unistd.h>
#include <errno.h>
#include <algorithm>

#include "event_selector.h"

FdHandler::~FdHandler()
{
    close(fd);
}

EventSelector::EventSelector() : descriptors(0)
{
    descriptors.reserve(expected_connections);
}

void EventSelector::add(FdHandler* handler)
{
    int fd = handler->get_fd();
    descriptors.push_back(handler);
    if (fd > max_fd)
        max_fd = fd;
}

//helper func for finding max descriptor
int get_max_fd(const std::vector<FdHandler*>& vec)
{
    int max = 0;
    int fd;
    for (auto& fdhandl : vec){
        fd = fdhandl->get_fd();
        if (max < fd)
            max = fd;
    }
    return max;
}

void EventSelector::remove(FdHandler* handler)
{
    int fd = handler->get_fd();
    auto item = std::find(descriptors.begin(), descriptors.end(), handler);
    if (item == descriptors.end())
        return;
    descriptors.erase(std::remove(descriptors.begin(), descriptors.end(), *item), descriptors.end());
    delete handler;
    if (fd == max_fd)
        max_fd = get_max_fd(descriptors);
}

//endless cycle using the system call "select"
void EventSelector::run()
{
    while(true)
    {
        fd_set read_s;
        FD_ZERO(&read_s);
        for (auto& fdhandl : descriptors){
            FD_SET(fdhandl->get_fd(), &read_s);
        }
        int res = select(max_fd + 1, &read_s, 0, 0, 0);
        if (res < 0){
            if (errno == EINTR)
                continue;
            else
                break;
        }
        if (res > 0){
            for (auto& fdhandl : descriptors){
                if(FD_ISSET(fdhandl->get_fd(), &read_s))
                    fdhandl->handle();
            }
        }
    }
}