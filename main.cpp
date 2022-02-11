#include "server.h"

static int port = 7000;

int main()
{
    EventSelector* selector = new EventSelector;
    Server* server = Server::start(selector, port);
    if(!server){
        perror("server");
        return 1;
    }
    selector->run();
    return 0;
}