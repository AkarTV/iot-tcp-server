#include <unistd.h>
#include <errno.h>
#include <algorithm>
#include <functional>

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

void EventSelector::remove(FdHandler* handler)
{
    int fd = handler->get_fd();
    auto item = std::find(descriptors.begin(), descriptors.end(), handler);
    if (item == descriptors.end())
        return;
    descriptors.erase(std::remove(descriptors.begin(), descriptors.end(), *item), descriptors.end());
    delete handler;
    if (fd == max_fd)
        max_fd = (*std::max_element(descriptors.cbegin(), descriptors.cend(), [](FdHandler* a, FdHandler* b){return a->get_fd() < b->get_fd();}))->get_fd();
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